#pragma once

#include "scene.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

struct TransformableComponent {
    glm::vec3 position;
    // glm::vec3 rotation; // quaternions?
    glm::vec3 scale;
};

struct RigidBodyComponent {
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

class Dynamics {
  public:
    void tick(Scene &scene) const;
};

} // namespace Physbuzz
