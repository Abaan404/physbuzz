#include "linear.hpp"

#include "../../dynamics.hpp"
#include <physbuzz/logging.hpp>
#include <physbuzz/renderer.hpp>

namespace Physbuzz {

LinearResolver::LinearResolver(float restitution) : m_Restitution(restitution) {
    if (m_Restitution > 1.0f) {
        Logger::WARNING("Linear restitution is greater than 1.0f");
    }
}

void LinearResolver::solve(Scene &scene, const Contact &contact) {
    Object &object1 = scene.getObject(contact.object1);
    Object &object2 = scene.getObject(contact.object2);

    const TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
    const TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

    // ensure the normal points from object1 -> object2
    const glm::vec3 direction = transform2.position - transform1.position;
    const glm::vec3 normal = glm::dot(contact.normal, direction) < 0.0f ? -contact.normal : contact.normal;

    RigidBodyComponent &rigidBody1 = object1.getComponent<RigidBodyComponent>();
    RigidBodyComponent &rigidBody2 = object2.getComponent<RigidBodyComponent>();

    // dont resolve if velocities are separating
    const float velocityAlongNormal = glm::dot(rigidBody2.velocity - rigidBody1.velocity, normal);
    if (velocityAlongNormal > 0) {
        return;
    }

    // calculate and apply impulses
    const float impulseScalar = (-(1 + m_Restitution) * velocityAlongNormal) / ((1 / rigidBody1.mass) + (1 / rigidBody2.mass));
    const glm::vec3 impulse = impulseScalar * normal;

    rigidBody1.velocity -= impulse / rigidBody1.mass;
    rigidBody2.velocity += impulse / rigidBody2.mass;
}

} // namespace Physbuzz
