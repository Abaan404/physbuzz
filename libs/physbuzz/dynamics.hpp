#pragma once

#include "clock.hpp"
#include "scene.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

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
    glm::vec3 accumTorques = {0.0f, 0.0f, 0.0f};

    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};

    struct {
        float inertia = 1.0f; // technically a Mz moment, Note: use a tensor for 3D
        float drag = 1.0f;
        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
    } angular;

    struct {
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
    } gravity;

    struct {
        float k1 = 0.0f;
        float k2 = 0.0f;
    } drag;

    void addForce(const glm::vec3 &force) {
        accumForces += force;
    }

    void addForceAtPoint(const glm::vec3 &force, const glm::vec3 &relPosition) {
        accumForces += force;
        accumTorques += glm::cross(relPosition, force);
    }
};

class Dynamics {
  public:
    Dynamics(float dtime);
    ~Dynamics();

    void tick(Scene &scene);
    const Clock &getClock() const;

    void translate(Object &object, const glm::vec3 &delta) const;
    void rotate(Object &object, const glm::quat &delta) const;

    const bool &isRunning() const;
    void start();
    void stop();
    const bool &toggle();

  private:
    void tickMotion(Object &object) const;

    float m_DeltaTime = 0.0f;
    bool m_IsRunning = true;
    Clock m_Clock;
};

} // namespace Physbuzz
