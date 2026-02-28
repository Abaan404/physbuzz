implementing "../lights";

namespace Physbuzz {

public struct PointLight : ILight {
    public const float4x4 projectionView[6];

    public const float3 position;
    public const float farPlane;
    public const float3 intensity;

    public float4 toLightSpace(float4 position, uint face) {
        return mul(position, this.projectionView[face]);
    }

    public float shadow(Geometry geometry, SamplerCube<float> depth) {
        constexpr float AMBIENT_SHADOW = 0.1;

        float3 fragToLight = geometry.position - this.position;

        float currentDepth = length(fragToLight);
        float closestDepth = depth.Sample(fragToLight).r * this.farPlane;

        if (currentDepth > closestDepth) {
            return 0.0f;
        }

        return 1.0f;
    }

    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 L = this.position - geometry.position;
        float distance = length(L);
        float3 incident = L / distance;

        float3 illuminance = this.intensity / (distance * distance);

        float NdotL = max(dot(geometry.normal, incident), 0.0f);

        return illuminance * brdf.evaluate(incident, viewDirection, geometry.normal) * NdotL;
    }
};

}
