#pragma once

#include "../../math/bounding.hpp"
#include "../../math/transform.hpp"
#include "../../resources/registry.hpp"
#include "../defines.hpp"

namespace Physbuzz {

class RigidBodyComponent {
  public:
    struct Info {
        Resource<PhysicsMesh> mesh;

        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
        float inverseMass = 0.0f;

        struct {
            glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
            glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
            glm::mat3 inverseInertia = {0.0f};

            float drag = 1.0f;
        } angular;

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

    void applyTransform(const Transform &transform);

    glm::vec3 calcSupportPoint(const glm::vec3 &direction) const;
    glm::vec3 calcSupportPoint(const glm::vec3 &direction, const Transform &transform) const;

    const Info &getInfo() const;
    const AABB &getAABB() const;

  private:
    Info m_Info;

    AABB m_AABB;
};

} // namespace Physbuzz
