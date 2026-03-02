implementing "../material";

namespace Physbuzz {

public struct SimpleMaterial : IMaterial {
    public const float3 diffuse;
    public const float specular;
    public const float specularity;

    public BlinnPhong prepare(Geometry geometry) {
        return {
            diffuse,
            specular,
            specularity
        };
    }
};

}
