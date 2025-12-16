implementing "../lights";

namespace Physbuzz {

struct DirectionalLight : ILight {
    float3 direction;
    float3 intensity;

    float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 incident = -this.direction;
        float NdotL = max(dot(geometry.normal, incident), 0.0f);

        return intensity * brdf.evaluate(incident, viewDirection, geometry.normal) * NdotL;
    }
};

}
