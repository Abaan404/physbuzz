module material;

import "brdf";
import "geometry";

__include "materials/simple";

namespace Physbuzz {

public interface IMaterial {
    public IBRDF prepare(Geometry geometry);
};

}
