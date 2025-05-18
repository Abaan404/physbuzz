#pragma once

#include "../resources/defines.hpp"

namespace Physbuzz {

struct OnResourceBuild {
    const ResourceID &identifier;
};

struct OnResourceDestroy {
    const ResourceID &identifier;
};

} // namespace Physbuzz
