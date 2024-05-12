#pragma once

#include "glm/ext/quaternion_trigonometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <physbuzz/clock.hpp>
#include <physbuzz/scene.hpp>

struct TransformableComponent {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {0.0f, 0.0f, 0.0f};
    glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    void translate(const glm::vec3 &delta) {
        position += delta;
    }
};

struct RigidBodyComponent {
    float mass = 1.0f;

    glm::vec3 accumForces = {0.0f, 0.0f, 0.0f};

    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};

    struct {
        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
    } orientation;

    struct {
        glm::vec3 acceleration = {0.0f, 1000.0f, 0.0f};
    } gravity;

    struct {
        float k1 = 0.0f;
        float k2 = 0.0f;
    } drag;

    void addForce(const glm::vec3 &force) {
        accumForces += force;
    }
};

class Dynamics {
  public:
    void tick(Physbuzz::Scene &scene);

    void translate(Physbuzz::Object &object, const glm::vec3 delta);
    void rotate(Physbuzz::Object &object, const glm::quat delta);

  private:
    void tickMotion(Physbuzz::Object &object);
};
