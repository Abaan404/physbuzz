module forward;

import "../lighting/brdf";
import "../lighting/lights";
import "../lighting/material";
import "../lighting/geometry";

namespace Physbuzz {

struct Camera {
    float3 position;
    float4x4 view;
    float4x4 projection;
};

struct Material {
    uint diffuseTextureId;
    uint specularTextureId;
    float specularity;

    static const uint STRIDE = 12;
};

struct InstanceData {
    float4x4 model;
    float4x4 invModel;
    uint materialIdx;
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
StructuredBuffer<InstanceData> models;

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord0 : TEXCOORD0;
};

struct CoarseVertex {
    float3 position;
    float3 normal;
    float2 texCoord0;
    uint objectId;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex : COARSE_VERTEX;
};

struct FSOutput {
    float4 pos;
    float4 normal;
    float4 albedo;
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

    output.coarseVertex.position = mul(float4(input.position, 1.0), instance.model).xyz;
    output.coarseVertex.normal = mul(input.normal, float3x3(instance.invModel));
    output.coarseVertex.texCoord0 = input.texCoord0;

    return output;
}

[shader("fragment")]
FSOutput fragMain(uint2 position: SV_Position, CoarseVertex coarseVertex: COARSE_VERTEX) {
    FSOutput output;

    InstanceData instance = models[coarseVertex.objectId];

    Material *boundMaterial = (Material *)(pc.materialBaseAddress + instance.materialIdx * Material.STRIDE);

    output.pos = { coarseVertex.position, 0.0f };
    output.normal = { coarseVertex.normal, 0.0f };
    output.albedo = { textures[boundMaterial.diffuseTextureId].Sample(coarseVertex.texCoord0).xyz,
                      textures[boundMaterial.specularTextureId].Sample(coarseVertex.texCoord0).x };

    return output;
}

}
