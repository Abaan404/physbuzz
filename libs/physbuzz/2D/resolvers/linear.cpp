#include "linear.hpp"

#include "../../dynamics.hpp"
#include <physbuzz/logging.hpp>
#include <physbuzz/renderer.hpp>

namespace Physbuzz {

LinearResolver2D::LinearResolver2D(Scene &scene, float restitution)
    : m_Restitution(restitution),
      ICollisionResolver(scene) {
    if (m_Restitution > 1.0f) {
        Logger::WARNING("Linear restitution is greater than 1.0f");
    }
}

void LinearResolver2D::solve(const Contact &contact) {
    Object &object1 = m_Scene.getObject(contact.object1);
    Object &object2 = m_Scene.getObject(contact.object2);

    RigidBodyComponent &body1 = object1.getComponent<RigidBodyComponent>();
    RigidBodyComponent &body2 = object2.getComponent<RigidBodyComponent>();

    const glm::vec3 impulse = calcImpulse(body1, body2, contact);

    body1.velocity -= impulse / body1.mass;
    body2.velocity += impulse / body2.mass;
}

const glm::vec3 LinearResolver2D::calcImpulse(const RigidBodyComponent &body1, const RigidBodyComponent &body2, const Contact &contact) {
    // dont resolve if velocities are separating
    const float velocityAlongNormal = glm::dot(body2.velocity - body1.velocity, contact.normal);
    if (velocityAlongNormal > 0) {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }

    return ((-(1.0f + m_Restitution) * velocityAlongNormal) / ((1.0f / body1.mass) + (1.0f / body2.mass))) * contact.normal;
}

} // namespace Physbuzz
