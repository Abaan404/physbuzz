#pragma once

#include "../../graphics/mesh.hpp"
#include "../../math/bounding.hpp"

namespace Physbuzz {

class RigidBodyComponent {
  public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };

    struct Geometry {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    };

    struct Info {
        Geometry geometry;

        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};

        struct {
            float inertia = 1.0f; // technically a Mz moment, Note: use a tensor for 3D
            float drag = 1.0f;
            glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
            glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
        } angular;

        float mass = 1.0f;

        glm::vec3 forces = {0.0f, 0.0f, 0.0f};
        glm::vec3 torques = {0.0f, 0.0f, 0.0f};

        struct {
            float k1 = 0.0f;
            float k2 = 0.0f;
        } drag;
    };

    RigidBodyComponent(const Info &info);

    void addForce(const glm::vec3 &force);
    void addForceAtPoint(const glm::vec3 &force, const glm::vec3 &relPosition);

    void update(const Info &info);

    const Info &getInfo() const;
    const AABB &getAABB() const;

  private:
    Info m_Info;

    AABB m_AABB;
};

} // namespace Physbuzz
