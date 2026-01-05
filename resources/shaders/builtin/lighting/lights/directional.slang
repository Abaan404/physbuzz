implementing "../lights";

namespace Physbuzz {

public struct DirectionalLight : ILight {
    float3 direction;
    float3 intensity;

    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 incident = -this.direction;
        float NdotL = max(dot(geometry.normal, incident), 0.0f);

        return this.intensity * brdf.evaluate(incident, viewDirection, geometry.normal) * NdotL;
    }
};

}
