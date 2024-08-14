#include "linear.hpp"

#include "../../dynamics.hpp"

namespace Physbuzz {

LinearResolver2D::LinearResolver2D(Scene *scene, float restitution)
    : ICollisionResolver(scene), m_Restitution(restitution) {
    if (m_Restitution > 1.0f) {
        Logger::WARNING("Linear restitution is greater than 1.0f");
    }
}

void LinearResolver2D::solve(const Contact &contact) {
    RigidBodyComponent &body1 = m_Scene->getComponent<RigidBodyComponent>(contact.object1);
    RigidBodyComponent &body2 = m_Scene->getComponent<RigidBodyComponent>(contact.object2);

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
