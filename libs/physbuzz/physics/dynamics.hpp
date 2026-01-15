#pragma once

#include "../ecs/system.hpp"
#include "../graphics/mesh.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

struct RigidBodyComponent {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };

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

    struct {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    } geometry = {};

    void addForce(const glm::vec3 &force) {
        accumForces += force;
    }

    void addForceAtPoint(const glm::vec3 &force, const glm::vec3 &relPosition) {
        accumForces += force;
        accumTorques += glm::cross(relPosition, force);
    }
};

class Dynamics : public System<RenderComponent, RigidBodyComponent> {
  public:
    Dynamics(float dtime);
    ~Dynamics();

    void tick();

    const bool &isRunning() const;
    void start();
    void stop();
    const bool &toggle();

  private:
    void tickMotion(ObjectID id) const;

    float m_DeltaTime = 0.0f;
    bool m_IsRunning = false;
};

} // namespace Physbuzz
