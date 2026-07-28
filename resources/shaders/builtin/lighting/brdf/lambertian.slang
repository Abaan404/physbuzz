implementing "../brdf";

import "../../defines.slang";

namespace Physbuzz {

public struct Lambertian : IBRDF {
    public float3 albedo;

    public float3 evaluate(float3 incident, float3 exitant, float3 normal) {
        return albedo / PI;
    }
};

}
