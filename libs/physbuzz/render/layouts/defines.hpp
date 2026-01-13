#pragma once

#include "../renderers/defines.hpp"
#include "physbuzz/debug/macros.hpp"
#include <cstddef>

namespace Physbuzz {

enum class LayoutLifetime {
    Global,
    PerFrame,
};

namespace detail {

constexpr std::size_t getLayoutLifetimeSetCount(LayoutLifetime lifetime) {
    switch (lifetime) {
    case LayoutLifetime::Global:
        return 1;
    case LayoutLifetime::PerFrame:
        return MAX_FRAMES_IN_FLIGHT;
    }

    PBZ_UNREACHABLE("[LayoutLifetime] No count specified for lifetime.");
}

}; // namespace detail

} // namespace Physbuzz
