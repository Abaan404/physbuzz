implementing "../lights";

namespace Physbuzz {

struct LightPair<T : ILight, U : ILight> : ILight {
    T first;
    U second;

    float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        return this.first.illuminate(geometry, brdf, viewDirection) + this.second.illuminate(geometry, brdf, viewDirection);
    }
};

}
