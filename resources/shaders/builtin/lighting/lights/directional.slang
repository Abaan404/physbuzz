implementing "../lights";

namespace Physbuzz {

public struct DirectionalLight : ILight {
    public const float4x4 projectionView;

    public const float3 direction;
    public const float3 intensity;

    public float4 toLightSpace(float4 position) {
        return mul(position, this.projectionView);
    };

    public float shadow(Geometry geometry, Sampler2D<float> depth) {
        constexpr float AMBIENT_SHADOW = 0.1;

        float4 projCoords = this.toLightSpace(float4(geometry.position, 1.0));
        projCoords.xyz /= projCoords.w;

        float currentDepth = projCoords.z;
        if (currentDepth > 1.0) {
            return 1.0f;
        }

        projCoords.xy = projCoords.xy * 0.5 + 0.5;
        float closestDepth = depth.Sample(projCoords.xy).r;

        if (currentDepth > closestDepth) {
            return AMBIENT_SHADOW;
        }

        return 1.0f;
    }

    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 incident = -this.direction;
        float NdotL = max(dot(geometry.normal, incident), 0.0f);

        return this.intensity * brdf.evaluate(incident, viewDirection, geometry.normal) * NdotL;
    }
};

}
