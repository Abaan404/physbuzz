#pragma once

#include "../resources/defines.hpp"
#include <filesystem>

namespace Physbuzz {

struct OnResourceBuild {
    const ResourceID &identifier;
};

struct OnResourceDestroy {
    const ResourceID &identifier;
};

struct OnResourceReload {
    const std::filesystem::path &filePath;
    WatchAction action;
};

} // namespace Physbuzz
