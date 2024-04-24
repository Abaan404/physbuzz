#pragma once

#include "../scene/scene.hpp"
#include <glm/glm.hpp>

struct TransformableComponent {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct RigidBodyComponent {
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

class Dynamics {
  public:
    Dynamics(Scene &scene);

    void tick();

  private:
    Scene &scene;
};
