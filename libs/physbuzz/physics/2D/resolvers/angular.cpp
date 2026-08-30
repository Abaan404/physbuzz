#include "angular.hpp"

#include "../../../ecs/scene.hpp"

namespace Physbuzz {

AngularResolver2D::AngularResolver2D(Scene *scene, float restitution)
    : LinearResolver2D(scene, restitution) {}

void AngularResolver2D::solve(const Contact &contact) {
    const auto [render1] = m_Scene->getComponent<RenderComponent>(contact.object1);
    const auto [render2] = m_Scene->getComponent<RenderComponent>(contact.object2);

    const auto [body1] = m_Scene->getComponent<RigidBodyComponent>(contact.object1);
    const auto [body2] = m_Scene->getComponent<RigidBodyComponent>(contact.object2);

    const glm::vec3 impulse = LinearResolver2D::calcImpulse(body1, body2, contact);

    const glm::vec3 torque1 = calcTorque(body1, render1.transform, contact.point1, impulse);
    const glm::vec3 torque2 = calcTorque(body2, render2.transform, contact.point2, impulse);

    // TODO this needs ALOT more work

    RigidBodyComponent::Info info1 = body1.getInfo();
    RigidBodyComponent::Info info2 = body2.getInfo();

    info1.velocity -= impulse / info1.mass;
    info2.velocity += impulse / info2.mass;

    info1.angular.velocity -= torque1 / info1.angular.inertia;
    info2.angular.velocity += torque2 / info2.angular.inertia;

    body1.update(info1);
    body2.update(info2);
}

const glm::vec3 AngularResolver2D::calcTorque(const RigidBodyComponent &body, const Transform &transfrom, const glm::vec3 &point, const glm::vec3 &impulse) {
    const glm::vec3 relPosition = point - transfrom.getInfo().position;
    return glm::cross(relPosition, impulse);
}

} // namespace Physbuzz
