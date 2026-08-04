module skybox;

import "../defines";
import "../spherical";

namespace Physbuzz {

static const float3 vertices[36] = {
    // Front
    float3(-1, -1, 1),
    float3(-1, 1, 1),
    float3(1, 1, 1),
    float3(-1, -1, 1),
    float3(1, 1, 1),
    float3(1, -1, 1),

    // Back
    float3(1, -1, -1),
    float3(1, 1, -1),
    float3(-1, 1, -1),
    float3(1, -1, -1),
    float3(-1, 1, -1),
    float3(-1, -1, -1),

    // Left
    float3(-1, -1, -1),
    float3(-1, 1, -1),
    float3(-1, 1, 1),
    float3(-1, -1, -1),
    float3(-1, 1, 1),
    float3(-1, -1, 1),

    // Right
    float3(1, -1, 1),
    float3(1, 1, 1),
    float3(1, 1, -1),
    float3(1, -1, 1),
    float3(1, 1, -1),
    float3(1, -1, -1),

    // Top
    float3(-1, 1, 1),
    float3(-1, 1, -1),
    float3(1, 1, -1),
    float3(-1, 1, 1),
    float3(1, 1, -1),
    float3(1, 1, 1),

    // Bottom
    float3(-1, -1, -1),
    float3(-1, -1, 1),
    float3(1, -1, 1),
    float3(-1, -1, -1),
    float3(1, -1, 1),
    float3(1, -1, -1)
};

[[vk::constant_id(0)]]
const bool isCubemap;

[[vk::binding(0, 0)]]
SamplerCube<float4> skybox_cube;

[[vk::binding(1, 0)]]
Sampler2D<float4> skybox_equirectangular;

[[vk::binding(0, 1)]]
ConstantBuffer<Camera> camera;

struct CoarseVertex {
    float3 texCoord;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex : COARSE_VERTEX;
};

[shader("vertex")]
VSOutput vertMain(uint vertexIdx: SV_VertexID) {
    VSOutput output;

    float3 position = vertices[vertexIdx];

    float4x4 viewNoTranslation = camera.view;
    viewNoTranslation[0][3] = 0.0f;
    viewNoTranslation[1][3] = 0.0f;
    viewNoTranslation[2][3] = 0.0f;

    output.sv_position = mul(mul(float4(position, 0.0f), viewNoTranslation), camera.projection).xyww;
    output.coarseVertex.texCoord = position;

    return output;
}

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) : SV_TARGET {
    float3 color = float3(0.0f);

    static if (isCubemap) {
        color = skybox_cube.Sample(coarseVertex.texCoord).rgb;
    }
    else {
        float2 uv = directionToUV(normalize(coarseVertex.texCoord));
        color = skybox_equirectangular.Sample(uv).rgb;
    }

    // tonemapping
    color = color / (color + 1.0f);

    return float4(color, 1.0f);
}

}
