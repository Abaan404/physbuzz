#pragma once

#include <glm/glm.hpp>
#include <physbuzz/scene.hpp>

struct TransformableComponent {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    // glm::vec3 rotation; // quaternions?
    glm::vec3 scale = {0.0f, 0.0f, 0.0f};
};

struct RigidBodyComponent {
    float mass = 1.0f;

    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
    glm::vec3 accumForces = {0.0f, 0.0f, 0.0f};

    struct {
        glm::vec3 acceleration = {0.0f, 9.8f, 0.0f};
    } gravity;

    struct {
        float k1 = 0.0f;
        float k2 = 0.0f;
    } drag;
};

class Dynamics {
  public:
    void tick(Physbuzz::Scene &scene);

    void displace(Physbuzz::Object &object, const glm::vec3 dDisplacement);
    void addForce(RigidBodyComponent &body, const glm::vec3 &force);

  private:
    void tickMotion(Physbuzz::Object &object);
};
