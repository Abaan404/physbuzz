#include "rigidbody.hpp"

namespace Physbuzz {

RigidBodyComponent::RigidBodyComponent(const Info &info)
    : m_Info(info) {
    AABB::Info aabb;

    for (const auto &vertex : m_Info.geometry.vertices) {
        aabb.min = glm::min(aabb.min, vertex.position);
        aabb.max = glm::max(aabb.max, vertex.position);
    }

    m_AABB = {aabb};
}

void RigidBodyComponent::addForce(const glm::vec3 &force) {
    m_Info.forces += force;
}

void RigidBodyComponent::addForceAtPoint(const glm::vec3 &force, const glm::vec3 &relPosition) {
    m_Info.forces += force;
    m_Info.torques += glm::cross(relPosition, force);
}

void RigidBodyComponent::update(const Info &info) {
    m_Info = info;
}

const RigidBodyComponent::Info &RigidBodyComponent::getInfo() const {
    return m_Info;
}

const AABB &RigidBodyComponent::getAABB() const {
    return m_AABB;
}

} // namespace Physbuzz
