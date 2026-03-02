implementing "../lights";

namespace Physbuzz {

public struct PointLight : ILight {
    public const float4x4 projectionView[6];

    public const float3 position;
    public const float farPlane;
    public const float3 intensity;

    private static const float3 sampleOffsetDirections[20] = {
        float3(1, 1, 1), float3(1, -1, 1), float3(-1, -1, 1), float3(-1, 1, 1),
        float3(1, 1, -1), float3(1, -1, -1), float3(-1, -1, -1), float3(-1, 1, -1),
        float3(1, 1, 0), float3(1, -1, 0), float3(-1, -1, 0), float3(-1, 1, 0),
        float3(1, 0, 1), float3(-1, 0, 1), float3(1, 0, -1), float3(-1, 0, -1),
        float3(0, 1, 1), float3(0, -1, 1), float3(0, -1, -1), float3(0, 1, -1)
    };

    public float shadow(Geometry geometry, SamplerCube<float> shadowMap) {
        static const float DISK_RADIUS = 1.5f;

        float shadowFactor = 0.0f;
        for (int i = 0; i < sampleOffsetDirections.getCount(); ++i) {
            shadowFactor += this.shadowProj(geometry, sampleOffsetDirections[i] * DISK_RADIUS, shadowMap);
        }

        return shadowFactor / sampleOffsetDirections.getCount();
    }

    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 L = this.position - geometry.position;
        float distance = length(L);
        float3 incident = L / distance;

        float3 illuminance = this.intensity / (distance * distance);

        float NdotL = max(dot(geometry.normal, incident), 0.0f);

        return illuminance * brdf.evaluate(incident, viewDirection, geometry.normal) * NdotL;
    }

    private float shadowProj(Geometry geometry, float3 offset, SamplerCube<float> shadowMap) {
        static const float AMBIENT_SHADOW = 0.1;

        float3 direction = geometry.position - this.position;

        // fix shadow acne with bias
        float bias = max(0.05f * (1.0f - dot(geometry.normal, -direction)), 0.005f);
        float depth = length(direction);

        float closestDepth = shadowMap.Sample(direction + offset).r * this.farPlane;

        float shadow = step(closestDepth, depth - bias);

        return lerp(1.0f, AMBIENT_SHADOW, shadow);
    }
}

}
