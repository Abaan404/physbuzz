module lights;

__include "lights/point";
__include "lights/directional";
__include "lights/spot";
__include "lights/empty";

import "geometry";
import "brdf";

namespace Physbuzz {

public interface ILight {
    public float3 illuminate<B : IBRDF>(Geometry geometry, B brdf, float3 viewDirection);
};

}
