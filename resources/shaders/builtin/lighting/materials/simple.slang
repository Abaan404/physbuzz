implementing "../material";

namespace Physbuzz {

public struct SimpleMaterial : IMaterial {
    public const float3 albedo;
    public const float metallic;
    public const float roughness;

    public CookTorrence prepare(Geometry geometry) {
        return {
            albedo,
            metallic,
            roughness
        };
    }
};

}
