#pragma once

#include <physbuzz/scene.hpp>
#include <glm/glm.hpp>

struct TransformableComponent {
    glm::vec3 position;
    // glm::vec3 rotation; // quaternions?
    glm::vec3 scale;
};

struct RigidBodyComponent {
    float mass;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

class Dynamics {
  public:
    void tick(Physbuzz::Scene &scene) const;
};
