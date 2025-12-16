implementing "../material";

namespace Physbuzz {

public struct SimpleMaterial : IMaterial {
    float3 diffuse;
    float3 specular;
    float specularity;

    public BlinnPhong prepare(Geometry geometry) {
        return {
            diffuse,
            specular,
            specularity
        };
    }
};

}

