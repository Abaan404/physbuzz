module brdf;

__include "brdf/blinnphong";
__include "brdf/cooktorrence";
__include "brdf/lambertian";

namespace Physbuzz {

public interface IBRDF {
    public float3 evaluate(float3 incident, float3 exitant, float3 normal);
};

}
