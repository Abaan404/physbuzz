implementing "../lights";

namespace Physbuzz {

public struct SpotLight : ILight {
    public const float4x4 projectionView;

    public const float3 position;
    public const float3 direction;
    public const float3 intensity;

    public const float cutOff;
    public const float outerCutOff;

    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 L = this.position - geometry.position;
        float distance = length(L);
        float3 incident = L / distance;

        float theta = dot(incident, -this.direction);
        float epsilon = this.cutOff - this.outerCutOff;
        float spotFactor = smoothstep(this.outerCutOff, this.cutOff, theta);

        float NdotL = max(dot(geometry.normal, incident), 0.0f);
        float3 illuminance = this.intensity / (distance * distance);

        return illuminance * brdf.evaluate(incident, viewDirection, geometry.normal) * NdotL * spotFactor;
    }
};

}
