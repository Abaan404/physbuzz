#pragma once

#include <physbuzz/graphics/mesh.hpp>
#include <physbuzz/math/bounding.hpp>

namespace Physbuzz {

struct PhysicsMesh {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };

    Geometry<Vertex> geometry;
    AABB bounding;

    std::function<glm::vec3(const glm::vec3 &)> supportPoint = nullptr;
};

template <>
struct IsResource<PhysicsMesh> : std::true_type {};

} // namespace Physbuzz
