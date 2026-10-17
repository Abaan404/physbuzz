#pragma once

#include "../resources/defines.hpp"

namespace Physbuzz {

struct OnResourceBuild {
    const ResourceID id;
};

struct OnResourceDestroy {
    const ResourceID id;
};

struct OnResourceReload {
    const ResourceID id;
};

} // namespace Physbuzz
