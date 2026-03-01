implementing "../lights";

namespace Physbuzz {

public struct DirectionalLight : ILight {
    public const float4x4 projectionView;

    public const float3 direction;
    public const float3 intensity;

    public float shadow(Geometry geometry, Sampler2D<float> shadowMap) {
        static const float SCALE = 1.5f;
        static const int RANGE = 1;
        static const int COUNT = (RANGE * 2 + 1) * (RANGE * 2 + 1);

        float2 texelSize;
        shadowMap.GetDimensions(texelSize.x, texelSize.y);
        texelSize = 1.0f / texelSize;

        float shadowFactor = 0.0f;
        for (int x = -RANGE; x <= RANGE; x++) {
            for (int y = -RANGE; y <= RANGE; y++) {
                float2 offset = texelSize * float2(x, y) * SCALE;

                shadowFactor += this.shadowProj(geometry, offset, shadowMap);
            }
        }

        return shadowFactor / COUNT;
    }

    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 incident = -this.direction;
        float NdotL = max(dot(geometry.normal, incident), 0.0f);

        return this.intensity * brdf.evaluate(incident, viewDirection, geometry.normal) * NdotL;
    }

    private float shadowProj(Geometry geometry, float2 offset, Sampler2D<float> shadowMap) {
        static const float AMBIENT_SHADOW = 0.1;

        float4 shadowClip = mul(float4(geometry.position, 1.0), this.projectionView);
        shadowClip.xyz /= shadowClip.w;

        // fix shadow acne with bias
        float bias = max(0.05f * (1.0f - dot(geometry.normal, -this.direction)), 0.005f);
        float depth = shadowClip.z;

        // outside clipped volume
        if (depth - bias > 1.0) {
            return 1.0f;
        }

        // get texcoords
        shadowClip.xy = shadowClip.xy * 0.5 + 0.5;
        float closestDepth = shadowMap.Sample(shadowClip.xy + offset).r;

        if (depth - bias > closestDepth) {
            return AMBIENT_SHADOW;
        }

        return 1.0f;
    }
}

}
