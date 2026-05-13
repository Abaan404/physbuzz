#pragma once

#include <cstddef>
#include <functional>

namespace Physbuzz {

template <class T>
inline std::size_t hashCombine(std::size_t seed, const T &v) {
    std::hash<T> hasher;
    return seed ^ (hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

} // namespace Physbuzz
