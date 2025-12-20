implementing "../lights";

namespace Physbuzz {

public struct LightArray<L : ILight, let N : int> : ILight {
    int count;
    L lights[N];

    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection) {
        float3 sum = 0;

        for (int i = 0; i < this.count; ++i) {
            sum += this.lights[i].illuminate(geometry, brdf, viewDirection);
        }

        return sum;
    }
};

}
