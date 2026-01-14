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

struct Light {
    LightArray<DirectionalLight, 5> directionals;
    LightArray<PointLight, 5> points;
    LightArray<SpotLight, 5> spots;
};

struct Material {
    uint diffuseTextureIds[5];
    uint specularTextureIds[5];
    float specularity;
};

struct InstanceData {
    float4x4 model;
    float4x4 invModel;
    uint64_t materialOffset;
};

struct PushConstants {
    uint64_t materialBaseAddress;
};

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
Sampler2D<float3> textures[];

[[vk::binding(0, 1)]]
ConstantBuffer<Camera> camera;

[[vk::binding(1, 1)]]
ConstantBuffer<Light> light;

[[vk::binding(0, 2)]]
StructuredBuffer<InstanceData> models;

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord0 : TEXCOORD0;
};

struct CoarseVertex {
    float3x3 TBN;
    float3 position;
    float3 normal;
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

    float3 T = mul(instance.model, float4(input.tangent, 0.0)).xyz;
    float3 N = mul(instance.model, float4(input.normal, 0.0)).xyz;
    float3 B = cross(N, T);

    output.coarseVertex.TBN = float3x3(T, B, N);
    output.coarseVertex.position = mul(float4(input.position, 1.0), instance.model).xyz;
    output.coarseVertex.normal = mul(input.normal, float3x3(instance.invModel));
    output.coarseVertex.texCoord0 = input.texCoord0;

    return output;
}

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) : SV_TARGET {
    InstanceData instance = models[coarseVertex.objectId];

    Geometry geometry = {
        coarseVertex.TBN,
        coarseVertex.position,
        coarseVertex.normal,
        coarseVertex.texCoord0
    };

    Material *boundMaterial = (Material *)(pc.materialBaseAddress + instance.materialOffset);

    SimpleMaterial material = {
        textures[boundMaterial.diffuseTextureIds[0]].Sample(coarseVertex.texCoord0),
        textures[boundMaterial.specularTextureIds[0]].Sample(coarseVertex.texCoord0),
        125.0f,
    };

    let brdf = material.prepare(geometry);

    float3 viewDirection = normalize(camera.position - geometry.position);

    float3 color = {};

    color += saturate(light.directionals.illuminate(geometry, brdf, viewDirection));
    color += saturate(light.points.illuminate(geometry, brdf, viewDirection));
    color += saturate(light.spots.illuminate(geometry, brdf, viewDirection));

    return float4(color, 1.0);
}

}
