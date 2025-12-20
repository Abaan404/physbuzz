module triangle;

struct VSInput {
    float3 inPosition : POSITION;
    float3 inColor : COLOR;
    float2 inTexCoord : TEXCOORD0;
};

struct CameraUBO {
    float3 position;
    float4x4 view;
    float4x4 projection;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUBO> ubo;

struct InstanceData {
    float4x4 model;
};

[[vk::binding(1, 0)]]
StructuredBuffer<InstanceData> models;

struct VSOutput {
    float4 pos : SV_Position;
    float3 fragColor : COLOR;
    float2 fragTexCoord : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    // TODO instancing
    float4x4 model = models[0].model;

    output.pos = mul(mul(mul(float4(input.inPosition, 1.0), model), ubo.view), ubo.projection);
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

[[vk::binding(0, 1)]]
Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    return texture.Sample(vertIn.fragTexCoord);
}
