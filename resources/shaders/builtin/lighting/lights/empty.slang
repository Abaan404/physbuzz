implementing "../lights";

namespace Physbuzz {

struct EmptyLight : ILight {
    float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        return {};
    }
};

}
