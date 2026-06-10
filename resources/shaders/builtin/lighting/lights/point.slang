implementing "../lights";

namespace Physbuzz {

public struct PointLight : ILight {
    static const float AMBIENT_SHADOW = 0.1;
    static const float NEAR_PLANE = 1.0f;
    static const float DISK_RADIUS = 1.5f;

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
        float3 direction = geometry.position - this.position + offset;

        // find facing depth
        float z = max(max(abs(direction.x), abs(direction.y)), abs(direction.z));

        // convert to linear range
        float invRange = 1.0f / max(this.farPlane - NEAR_PLANE, 1e-5f);
        float depth = saturate((this.farPlane * invRange) - ((this.farPlane * NEAR_PLANE) * invRange) / max(z, 1e-5f));

        float closestDepth = shadowMap.Sample(direction).r;

        float shadow = step(closestDepth, depth);

        return lerp(1.0f, AMBIENT_SHADOW, shadow);
    }
}

}
