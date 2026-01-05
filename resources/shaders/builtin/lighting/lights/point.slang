implementing "../lights";

namespace Physbuzz {

public struct PointLight : ILight {
    float3 position;
    float3 intensity;

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
