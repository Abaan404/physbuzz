#include "angular.hpp"

namespace Physbuzz {

AngularResolver2D::AngularResolver2D(Scene *scene, float restitution)
    : LinearResolver2D(scene, restitution) {}

void AngularResolver2D::solve(const Contact &contact) {
    const TransformableComponent &transform1 = m_Scene->getComponent<TransformableComponent>(contact.object1);
    const TransformableComponent &transform2 = m_Scene->getComponent<TransformableComponent>(contact.object2);

    RigidBodyComponent &body1 = m_Scene->getComponent<RigidBodyComponent>(contact.object1);
    RigidBodyComponent &body2 = m_Scene->getComponent<RigidBodyComponent>(contact.object2);

    const glm::vec3 impulse = LinearResolver2D::calcImpulse(body1, body2, contact);

    const glm::vec3 torque1 = calcTorque(body1, transform1, contact.point1, impulse);
    const glm::vec3 torque2 = calcTorque(body2, transform2, contact.point2, impulse);

    // TODO this needs ALOT more work

    body1.velocity -= impulse / body1.mass;
    body2.velocity += impulse / body2.mass;

    body1.angular.velocity -= torque1 / body1.angular.inertia;
    body2.angular.velocity += torque2 / body2.angular.inertia;
}

const glm::vec3 AngularResolver2D::calcTorque(const RigidBodyComponent &body, const TransformableComponent &transform, const glm::vec3 &point, const glm::vec3 &impulse) {
    const glm::vec3 relPosition = point - transform.position;
    return glm::cross(relPosition, impulse);
}

} // namespace Physbuzz
