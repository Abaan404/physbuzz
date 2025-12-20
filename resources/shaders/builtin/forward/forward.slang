module forward;

import "../lighting/brdf";
import "../lighting/lights";
import "../lighting/material";
import "../lighting/geometry";

namespace Physbuzz {

struct CameraUBO {
    float4x4 view;
    float4x4 projection;
    float3 position;
};

struct InstanceData {
    float4x4 model;
    float4x4 invModel;
};

ILight gLight;
IMaterial gMaterial;

[[vk::binding(1, 0)]]
StructuredBuffer<InstanceData> models;

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUBO> ubo;

struct CoarseVertex {
    float3x3 TBN;
    float3 position;
    float3 normal;
    float2 texCoord0;
};

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord0 : TEXCOORD0;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex : COARSE_VERTEX;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    // TODO instancing
    float4x4 model = models[0].model;
    float4x4 invModel = models[0].invModel;

    output.sv_position = mul(ubo.projection * ubo.view * model, float4(input.position, 1.0));

    float3 T = mul(model, float4(input.tangent, 0.0)).xyz;
    float3 N = mul(model, float4(input.normal, 0.0)).xyz;
    float3 B = cross(N, T);

    output.coarseVertex.TBN = float3x3(T, B, N);
    output.coarseVertex.position = mul(model, float4(input.position, 1.0)).xyz;
    output.coarseVertex.normal = mul(float3x3(transpose(invModel)), input.normal);
    output.coarseVertex.texCoord0 = input.texCoord0;

    return output;
}

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) : SV_TARGET {
    Geometry geometry = {
        coarseVertex.TBN,
        coarseVertex.position,
        coarseVertex.normal,
        coarseVertex.texCoord0
    };

    let brdf = gMaterial.prepare(geometry);

    float3 viewDirection = normalize(ubo.position - geometry.position);
    float3 color = saturate(gLight.illuminate(geometry, brdf, viewDirection));

    return float4(color, 1.0);
}

}
