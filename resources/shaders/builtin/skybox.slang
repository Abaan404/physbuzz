module skybox;

namespace Physbuzz {

struct Camera {
    float3 position;
    float4x4 view;
    float4x4 projection;
};

[[vk::binding(0, 0)]]
SamplerCube<float4> skybox;

[[vk::binding(0, 1)]]
ConstantBuffer<Camera> camera;

struct VSInput {
    float3 position : POSITION;
};

struct CoarseVertex {
    float3 texCoord;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex : COARSE_VERTEX;
};

[shader("vertex")]
VSOutput vsMain(VSInput input) {
    VSOutput output;

    float4x4 viewNoTranslation = camera.view;
    viewNoTranslation[0][3] = 0.0f;
    viewNoTranslation[1][3] = 0.0f;
    viewNoTranslation[2][3] = 0.0f;

    output.sv_position = mul(mul(float4(input.position, 0.0f), viewNoTranslation), camera.projection).xyww;
    output.coarseVertex.texCoord = input.position;

    return output;
}

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) : SV_TARGET {
    return skybox.Sample(coarseVertex.texCoord);
}

}
