module forward;

import "lighting/brdf";
import "lighting/lights";
import "lighting/material";
import "lighting/geometry";

namespace Physbuzz {

struct Camera {
    float3 position;
    float4x4 view;
    float4x4 projection;
};

struct Material {
    uint diffuseTextureId;
    uint specularTextureId;
    uint heightTextureId;
    float specularity;

    static const uint STRIDE = 16;
};

struct InstanceData {
    float4x4 model;
    float4x4 normal;
    uint materialIdx;
};

struct PushConstants {
    uint directionalCount;
    uint spotCount;
    uint pointCount;

    uint64_t materialBaseAddress;
};

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
Sampler2D<float3> textures[];

[[vk::binding(0, 1)]]
ConstantBuffer<Camera> camera;

[[vk::binding(1, 1)]]
StructuredBuffer<DirectionalLight> directionals;

[[vk::binding(2, 1)]]
StructuredBuffer<PointLight> points;

[[vk::binding(3, 1)]]
StructuredBuffer<SpotLight> spots;

[[vk::binding(4, 1)]]
StructuredBuffer<InstanceData> models;

[[vk::binding(5, 1)]]
Sampler2D<float> directionalDepth;

[[vk::binding(6, 1)]]
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
    output.coarseVertex.objectId = instanceId + baseInstance;

    InstanceData instance = models[output.coarseVertex.objectId];
    output.sv_position = mul(mul(mul(float4(input.position, 1.0), instance.model), camera.view), camera.projection);

    output.coarseVertex.position = mul(float4(input.position, 1.0), instance.model).xyz;
    output.coarseVertex.normal = normalize(mul(input.normal, float3x3(instance.normal)));
    output.coarseVertex.tangent = normalize(mul(input.tangent, float3x3(instance.model)));
    output.coarseVertex.bitangent = cross(output.coarseVertex.normal, output.coarseVertex.tangent);
    output.coarseVertex.texCoord0 = input.texCoord0;

    return output;
}

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) : SV_TARGET {
    InstanceData instance = models[coarseVertex.objectId];

    Geometry geometry = {
        coarseVertex.position,
        coarseVertex.normal,
    };

    Material *boundMaterial = (Material *)(pc.materialBaseAddress + instance.materialIdx * Material.STRIDE);

    float3 albedo = float3(0.0f);
    float specular = 0.0f;

    if (boundMaterial.diffuseTextureId != -1) {
        albedo = textures[boundMaterial.diffuseTextureId].Sample(coarseVertex.texCoord0).rgb;
    }

    if (boundMaterial.specularTextureId != -1) {
        specular = textures[boundMaterial.specularTextureId].Sample(coarseVertex.texCoord0).r;
    }

    if (boundMaterial.heightTextureId != -1) {
        float3x3 TBN = {
            coarseVertex.tangent,
            coarseVertex.bitangent,
            coarseVertex.normal,
        };

        geometry.normal = textures[boundMaterial.heightTextureId].Sample(coarseVertex.texCoord0).rgb;
        geometry.normal = geometry.normal * 2.0 - 1.0;
        geometry.normal = normalize(mul(geometry.normal, TBN));
    }

    SimpleMaterial material = {
        albedo,
        specular,
        125.0f,
    };

    let brdf = material.prepare(geometry);

    float3 viewDirection = normalize(camera.position - geometry.position);

    float3 color = {};

    for (int i = 0; i < pc.directionalCount; i++) {
        color += directionals[i].shadow(geometry, directionalDepth) * directionals[i].illuminate(geometry, brdf, viewDirection);
    }

    for (int i = 0; i < pc.pointCount; i++) {
        color += points[i].shadow(geometry, pointDepth) * points[i].illuminate(geometry, brdf, viewDirection);
    }

    for (int i = 0; i < pc.spotCount; i++) {
        color += spots[i].illuminate(geometry, brdf, viewDirection);
    }

    return float4(color, 1.0);
}

}
