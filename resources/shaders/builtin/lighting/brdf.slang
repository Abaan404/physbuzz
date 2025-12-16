module brdf;

__include "brdf/blinnphong";

namespace Physbuzz {

public interface IBRDF {
    public float3 evaluate(float3 incident, float3 exitant, float3 normal);
};

}
