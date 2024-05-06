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
    void tick(Physbuzz::Scene &scene);

  private:
    void motion(Physbuzz::Object &object);
    void gravity(Physbuzz::Object &object);

    glm::vec3 m_Gravity = glm::vec3(0.0f, 0.001f, 0.0f);
};
