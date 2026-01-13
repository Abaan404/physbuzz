implementing "../material";

namespace Physbuzz {

public struct SimpleMaterial : IMaterial {
    public float3 diffuse;
    public float3 specular;
    public float specularity;

    public BlinnPhong prepare(Geometry geometry) {
        return {
            diffuse,
            specular,
            specularity
        };
    }
};

}
