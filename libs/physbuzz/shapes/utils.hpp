#pragma once

#include "../graphics/mesh.hpp"
#include <vector>

namespace Physbuzz {

namespace detail {

struct NormalTangent {
    glm::vec3 normal;
    glm::vec3 tangent;
};

std::vector<glm::vec2> generate2DTexCoords(const std::vector<glm::vec3> &positions);
std::vector<NormalTangent> generate2DNormalTangent(const std::vector<Physbuzz::Index> &indices, const std::vector<glm::vec3> &positions, const std::vector<glm::vec2> &texCoords);

} // namespace detail

} // namespace Physbuzz
