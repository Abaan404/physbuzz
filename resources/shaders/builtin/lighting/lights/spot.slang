implementing "../lights";

namespace Physbuzz {

public struct SpotLight : ILight {
    float3 position;
    float3 direction;
    float3 intensity;

    float cutOff;
    float outerCutOff;

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
