module irradiance;

import "../defines";
import "../spherical";

namespace Physbuzz {

[[vk::constant_id(0)]]
const bool isCubemap;

[[vk::binding(0, 0)]]
SamplerCube<float4> input_cube;

[[vk::binding(1, 0)]]
Sampler2D<float4> input_equirectangular;

[[vk::binding(2, 0)]]
RWTexture2D<float4> output;

static const float sampleDelta = 0.025;

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 tid: SV_DispatchThreadID) {
    uint width, height;
    output.GetDimensions(width, height);

    float2 uv = (float2(tid.xy) + 0.5f) / float2(width, height);
    float3 N = uvToDirection(uv);

    // figure out the orientation of the normal
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float3 irradiance = float3(0.0f);
    float sampleCount = 0.0f;

    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta) {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta) {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            static if (isCubemap) {
                irradiance += input_cube.Sample(sampleVec).rgb * cos(theta) * sin(theta);
            }

            else {
                float2 uvSample = directionToUV(sampleVec);
                irradiance += input_equirectangular.Sample(uvSample).rgb * cos(theta) * sin(theta);
            }

            sampleCount++;
        }
    }

    // evaluate total irradiance
    irradiance = PI * irradiance * (1.0f / sampleCount);
    output.Store(tid.xy, float4(irradiance, 1.0f));
}

}
