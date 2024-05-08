#pragma once

#include <physbuzz/clock.hpp>
#include <glm/glm.hpp>
#include <physbuzz/scene.hpp>

struct TransformableComponent {
    glm::vec3 position{};
    // glm::vec3 rotation; // quaternions?
    glm::vec3 scale{};
};

struct RigidBodyComponent {
    float mass{1.0f};

    glm::vec3 velocity{};
    glm::vec3 acceleration{};
    glm::vec3 accumForces{};
};

struct GravityComponent {
    glm::vec3 gravity{0.0f, 9.8f, 0.0f};
};

struct DragComponent {
    float k1{0.0f};
    float k2{0.0f};
};

class Dynamics {
  public:
    void tick(Physbuzz::Scene &scene);

    void displace(Physbuzz::Object &object, const glm::vec3 dDisplacement);
    void addForce(RigidBodyComponent &body, const glm::vec3 &force);

  private:
    void motion(Physbuzz::Object &object);
};
