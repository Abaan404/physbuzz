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

struct PushConstants {
    uint directionalCount;
    uint spotCount;
    uint pointCount;
};

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
ConstantBuffer<Camera> camera;

[[vk::binding(1, 0)]]
StructuredBuffer<DirectionalLight> directionals;

[[vk::binding(2, 0)]]
StructuredBuffer<PointLight> points;

[[vk::binding(3, 0)]]
StructuredBuffer<SpotLight> spots;

[[vk::input_attachment_index(0)]]
[[vk::binding(4, 0)]]
SubpassInput<float4> gBuffer0;

[[vk::input_attachment_index(1)]]
[[vk::binding(5, 0)]]
SubpassInput<float4> gBuffer1;

[[vk::input_attachment_index(2)]]
[[vk::binding(6, 0)]]
SubpassInput<float4> gBuffer2;

[[vk::binding(7, 0)]]
Sampler2D<float> directionalDepth;

[[vk::binding(8, 0)]]
SamplerCube<float> pointDepth;

struct CoarseVertex {
    float2 texCoord;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex : COARSE_VERTEX;
};

[shader("vertex")]
VSOutput vertMain(uint vertexIdx: SV_VertexID) {
    VSOutput output;
    output.coarseVertex.texCoord = float2((vertexIdx << 1) & 2, vertexIdx & 2);
    output.sv_position = float4(output.coarseVertex.texCoord * 2.0f - 1.0f, 0.0f, 1.0f);
    return output;
}

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) {
    Geometry geometry = {
        gBuffer0.SubpassLoad().xyz,
        gBuffer1.SubpassLoad().xyz,
    };

    SimpleMaterial material = {
        gBuffer2.SubpassLoad().rgb,
        gBuffer2.SubpassLoad().a,
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

    return float4(color, 1.0f);
}

}
