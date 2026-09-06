#include "rigidbody.hpp"

namespace Physbuzz {

RigidBodyComponent::RigidBodyComponent(const Info &info)
    : m_Info(info) {
    m_AABB = info.mesh->bounding;
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

void RigidBodyComponent::applyTransform(const Transform &transform) {
    m_AABB = m_Info.mesh->bounding.withTransform(transform);
}

} // namespace Physbuzz
