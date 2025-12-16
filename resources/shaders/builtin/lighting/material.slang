module material;

import "brdf";
import "geometry";

namespace Physbuzz {

public interface IMaterial {
    public IBRDF prepare(Geometry geometry);
};

}
