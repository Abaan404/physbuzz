#include "simple.hpp"

#include "../../ecs/scene.hpp"
#include "../../render/defines.hpp"
#include "../components/rigidbody.hpp"

namespace Physbuzz {

SimpleResolver3D::SimpleResolver3D(Scene *scene, const Info &info)
    : CollisionResolver(scene), m_Info(info) {}

bool SimpleResolver3D::buildImpl() {
    return true;
}

bool SimpleResolver3D::destroyImpl() {
    return true;
}

// ref: https://www.sciencedirect.com/science/chapter/monograph/abs/pii/B9780124158429000071
void SimpleResolver3D::solveImpl(const Contact &contact) {
    const auto [transform1] = m_Scene->getComponent<TransformComponent>(contact.object1);
    const auto [transform2] = m_Scene->getComponent<TransformComponent>(contact.object2);

    const auto [body1] = m_Scene->getComponent<RigidBodyComponent>(contact.object1);
    const auto [body2] = m_Scene->getComponent<RigidBodyComponent>(contact.object2);

    RigidBodyComponent::Info info1 = body1.getInfo();
    RigidBodyComponent::Info info2 = body2.getInfo();

    glm::vec3 N = -contact.normal;

    glm::vec3 r1 = contact.point - transform1.transform.getInfo().position;
    glm::vec3 r2 = contact.point - transform2.transform.getInfo().position;

    glm::vec3 pointVelocity1 = info1.velocity + glm::cross(info1.angular.velocity, r1);
    glm::vec3 pointVelocity2 = info2.velocity + glm::cross(info2.angular.velocity, r2);

    float speedRelative = glm::dot(pointVelocity1 - pointVelocity2, N);

    // dont resolve if velocities are separating
    if (speedRelative >= 0.0f) {
        return;
    }

    glm::mat3 &inverseInertia1 = info1.angular.inverseInertia;
    glm::mat3 &inverseInertia2 = info2.angular.inverseInertia;

    float angular1 = glm::dot(N, glm::cross(inverseInertia1 * glm::cross(r1, N), r1));
    float angular2 = glm::dot(N, glm::cross(inverseInertia2 * glm::cross(r2, N), r2));

    const float denominator = info1.inverseMass + info2.inverseMass + angular1 + angular2;
    glm::vec3 impulse = -(1.0f + m_Info.restitution) * speedRelative / (info1.inverseMass + info2.inverseMass + angular1 + angular2) * N;

    info1.velocity += impulse * info1.inverseMass;
    info2.velocity -= impulse * info2.inverseMass;

    info1.angular.velocity += inverseInertia1 * glm::cross(r1, impulse);
    info2.angular.velocity -= inverseInertia2 * glm::cross(r2, impulse);

    body1.update(info1);
    body2.update(info2);
}

} // namespace Physbuzz
