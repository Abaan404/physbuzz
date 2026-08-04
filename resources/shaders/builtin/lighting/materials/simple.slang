implementing "../material";

namespace Physbuzz {

public struct SimpleMaterial : IMaterial {
    public const float3 albedo;
    public const float3 irradiance;
    public const float3 prefilteredColor;
    public const float2 envBRDF;
    public const float metallic;
    public const float roughness;
    public const float ambientOcclusion;

    public CookTorrence prepare(Geometry geometry) {
        return {
            albedo,
            irradiance,
            prefilteredColor,
            envBRDF,
            metallic,
            roughness,
            ambientOcclusion,
        };
    }
};

}
