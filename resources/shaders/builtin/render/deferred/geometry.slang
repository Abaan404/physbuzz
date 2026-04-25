module forward;

import "../../defines";
import "../../lighting/brdf";
import "../../lighting/lights";
import "../../lighting/material";
import "../../lighting/geometry";

namespace Physbuzz {

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
StructuredBuffer<InstanceData> objects;

[[vk::binding(2, 1)]]
StructuredBuffer<VisibilityData> visibleInstances;

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

struct FSOutput {
    float4 position;
    float4 normal;
    float4 albedo;
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
FSOutput fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) {
    FSOutput output;

    InstanceData object = objects[coarseVertex.objectId];

    Material *boundMaterial = (Material *)(pc.materialBaseAddress + object.materialIdx * Material.STRIDE);

    float3 albedo = float3(0.0f);
    float3 normal = coarseVertex.normal;
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

        normal = textures[boundMaterial.heightTextureId].Sample(coarseVertex.texCoord0).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(mul(normal, TBN));
    }

    output.position = { coarseVertex.position, 1.0f };
    output.normal = { normal, 1.0f };
    output.albedo = { albedo, specular };

    return output;
}

}
