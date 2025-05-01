#pragma once

#include "../ecs/scene.hpp"
#include "../misc/clock.hpp"
#include "../render/model.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

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

class Dynamics : public System<TransformComponent, ModelComponent, RigidBodyComponent> {
  public:
    Dynamics(float dtime);
    ~Dynamics();

    void tick(Scene &scene);
    const Clock &getClock() const;

    const bool &isRunning() const;
    void start();
    void stop();
    const bool &toggle();

  private:
    void tickMotion(Scene &scene, ObjectID id) const;

    float m_DeltaTime = 0.0f;
    bool m_IsRunning = false;
    Clock m_Clock;
};

} // namespace Physbuzz
