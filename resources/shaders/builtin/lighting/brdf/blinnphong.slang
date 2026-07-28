implementing "../brdf";

namespace Physbuzz {

public struct BlinnPhong : IBRDF {
    public const float3 kd;
    public const float ks;
    public const float shininess;

    public float3 evaluate(float3 incident, float3 exitant, float3 normal) {
        float3 halfway = normalize(exitant + incident);

        float NdotL = saturate(dot(normal, incident));
        float NdotH = saturate(dot(normal, halfway));

        float3 diffuse = kd;
        float3 specular = ks * pow(NdotH, shininess);

        return diffuse + specular;
    }
};

}
