module forward;

import "../defines";
import "../spherical";
import "../lighting/lights";
import "../lighting/material";
import "../lighting/geometry";

namespace Physbuzz {

struct PushConstants {
    uint directionalCount;
    uint spotCount;
    uint pointCount;

    uint64_t materialBaseAddress;
};

[[vk::constant_id(0)]]
const bool enableShadows;

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
Sampler2D<float3> textures[];

[[vk::binding(0, 1)]]
Sampler2D<float4> irradianceMap;

[[vk::binding(1, 1)]]
Sampler2D<float4> prefilterMap;

[[vk::binding(2, 1)]]
Sampler2D<float4> brdfLut;

[[vk::binding(0, 2)]]
ConstantBuffer<Camera> camera;

[[vk::binding(1, 2)]]
StructuredBuffer<DirectionalLight> directionals;

[[vk::binding(2, 2)]]
StructuredBuffer<PointLight> points;

[[vk::binding(3, 2)]]
StructuredBuffer<SpotLight> spots;

[[vk::binding(4, 2)]]
StructuredBuffer<InstanceData> objects;

[[vk::binding(5, 2)]]
StructuredBuffer<VisibilityData> visibleInstances;

[[vk::binding(6, 2)]]
Sampler2D<float> directionalDepth;

[[vk::binding(7, 2)]]
SamplerCube<float> pointDepth;

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord0 : TEXCOORD0;
};

struct CoarseVertex {
    float3 position;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float2 texCoord0;
    uint objectId;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex : COARSE_VERTEX;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation) {
    VSOutput output;
    output.coarseVertex.objectId = visibleInstances[instanceId + baseInstance].instanceId;

    InstanceData object = objects[output.coarseVertex.objectId];
    output.sv_position = mul(mul(mul(float4(input.position, 1.0), object.model), camera.view), camera.projection);

    output.coarseVertex.position = mul(float4(input.position, 1.0), object.model).xyz;
    output.coarseVertex.normal = normalize(mul(input.normal, float3x3(object.normal)));
    output.coarseVertex.tangent = normalize(mul(input.tangent, float3x3(object.model)));
    output.coarseVertex.bitangent = cross(output.coarseVertex.normal, output.coarseVertex.tangent);
    output.coarseVertex.texCoord0 = input.texCoord0;

    return output;
}

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) : SV_TARGET {
    InstanceData object = objects[coarseVertex.objectId];

    Geometry geometry = {
        coarseVertex.position,
        coarseVertex.normal,
    };

    Material *boundMaterial = (Material *)(pc.materialBaseAddress + object.materialIdx * Material.STRIDE);

    float metallic = 1.0f;
    float roughness = 1.0f;
    float ambientOcclusion = 1.0f;
    float3 irradiance = 1.0f;
    float3 albedo = float3(1.0f);

    if (boundMaterial.albedoTextureId != -1) {
        albedo *= textures[boundMaterial.albedoTextureId].Sample(coarseVertex.texCoord0).rgb;
    }

    if (boundMaterial.roughnessTextureId != -1) {
        roughness *= textures[boundMaterial.roughnessTextureId].Sample(coarseVertex.texCoord0).g;
    }

    if (boundMaterial.metallicTextureId != -1) {
        metallic *= textures[boundMaterial.metallicTextureId].Sample(coarseVertex.texCoord0).b;
    }

    if (boundMaterial.normalTextureId != -1) {
        float3x3 TBN = {
            coarseVertex.tangent,
            coarseVertex.bitangent,
            coarseVertex.normal,
        };

        geometry.normal = textures[boundMaterial.normalTextureId].Sample(coarseVertex.texCoord0).rgb;
        geometry.normal = geometry.normal * 2.0 - 1.0;
        geometry.normal = normalize(mul(geometry.normal, TBN));
    }

    irradiance *= irradianceMap.Sample(directionToUV(normalize(coarseVertex.normal))).rgb;
    metallic *= boundMaterial.metallicFactor;
    roughness *= boundMaterial.roughnessFactor;
    albedo *= boundMaterial.albedoFactor;

    SimpleMaterial material = {
        albedo,
        irradiance,
        metallic,
        roughness,
        ambientOcclusion,
    };

    let brdf = material.prepare(geometry);

    float3 viewDirection = normalize(camera.position - geometry.position);

    float3 color = {};

    for (int i = 0; i < pc.directionalCount; i++) {
        color += directionals[i].illuminate(geometry, brdf, viewDirection);
    }

    for (int i = 0; i < pc.pointCount; i++) {
        color += points[i].illuminate(geometry, brdf, viewDirection);
    }

    for (int i = 0; i < pc.spotCount; i++) {
        color += spots[i].illuminate(geometry, brdf, viewDirection);
    }

    color += brdf.evaluateIndirect(viewDirection, geometry.normal);

    static if (enableShadows) {
        for (int i = 0; i < pc.directionalCount; i++) {
            color *= directionals[i].shadow(geometry, directionalDepth);
        }

        for (int i = 0; i < pc.pointCount; i++) {
            color *= points[i].shadow(geometry, pointDepth);
        }
    }

    return float4(color, 1.0);
}

}
