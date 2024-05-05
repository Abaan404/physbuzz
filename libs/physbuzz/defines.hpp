#pragma once

#include <concepts>
#include <cstdint>

namespace Physbuzz {

template <typename T>
concept Comparable = requires(T a, T b) {
    a <=> b;
};

template <typename F, typename T>
concept Comparator = requires(F f, T &t1, T &t2) {
    { f(t1, t2) } -> std::convertible_to<bool>;
};

using ObjectID = std::uint32_t;

} // namespace Physbuzz
