module prefilter;

import "../defines";
import "../spherical";
import "../sampling";

namespace Physbuzz {

struct PushConstants {
    uint mipLevel;
    float roughness;
}

[[vk::push_constant]]
PushConstants pc;

[[vk::constant_id(0)]]
const bool isCubemap;

[[vk::binding(0, 0)]]
SamplerCube<float4> input_cube;

[[vk::binding(1, 0)]]
Sampler2D<float4> input_equirectangular;

[[vk::binding(2, 0)]]
RWTexture2D<float4> output[];

static const float sampleDelta = 0.025;
static const uint maxSampleCount = 1024u;

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 tid: SV_DispatchThreadID) {
    uint width, height;
    output[pc.mipLevel].GetDimensions(width, height);

    if (tid.x >= width || tid.y >= height) {
        return;
    }

    float2 uv = (float2(tid.xy) + 0.5f) / float2(width, height);
    float3 N = uvToDirection(uv);
    float3 R = N;
    float3 V = R;

    float totalWeight = 0.0;
    float3 prefilteredColor = float3(0.0);

    for (uint i = 0u; i < maxSampleCount; ++i) {
        float2 Xi = Hammersley(i, maxSampleCount);
        float3 H = ImportanceSampleGGX(Xi, N, pc.roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            float2 uvSample = directionToUV(L);

            static if (isCubemap) {
                prefilteredColor += input_cube.SampleLevelZero(L).rgb * NdotL;
            }

            else {
                float2 uvSample = directionToUV(L);
                prefilteredColor += input_equirectangular.SampleLevelZero(uvSample).rgb * NdotL;
            }

            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    output[pc.mipLevel].Store(tid.xy, float4(prefilteredColor.rgb, 1.0f));
}

}
