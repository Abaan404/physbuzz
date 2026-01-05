implementing "../brdf";

namespace Physbuzz {

public struct BlinnPhong : IBRDF {
    public float3 kd;
    public float3 ks;
    public float shininess;

    public float3 evaluate(float3 incident, float3 exitant, float3 normal) {
        float3 diffuse = kd;

        float nDotH = saturate(dot(normal, normalize(exitant + incident)));
        float3 specular = ks * pow(nDotH, shininess);

        return diffuse + specular;
    }
};

}
