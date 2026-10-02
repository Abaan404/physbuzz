#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

inline glm::vec3 perpendicular(const glm::vec3 &vec) {
    // https://math.stackexchange.com/a/4112622
    return glm::vec3(
        std::copysign(vec.z, vec.x),
        std::copysign(vec.z, vec.y),
        -std::copysign(std::abs(vec.x) + std::abs(vec.y), vec.z));
}

template <float epsilon = 0.0f>
inline bool isFacing(const glm::vec3 &vec, const glm::vec3 &direction) {
    return glm::dot(vec, direction) > epsilon;
}

} // namespace Physbuzz
