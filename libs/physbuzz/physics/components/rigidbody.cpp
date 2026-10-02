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

void RigidBodyComponent::applyTransform(const Transform &transform) {
    m_AABB = m_Info.mesh->bounding.withTransform(transform);
}

glm::vec3 RigidBodyComponent::calcSupportPoint(const glm::vec3 &direction) const {
    if (m_Info.mesh->supportPoint) {
        return m_Info.mesh->supportPoint(direction);
    }

    glm::vec3 point;
    float proj = std::numeric_limits<float>::lowest();

    for (const auto &vertex : m_Info.mesh->geometry.vertices) {
        const float newProj = glm::dot(vertex.position, direction);
        if (newProj > proj) {
            point = vertex.position;
            proj = newProj;
        }
    }

    return point;
}

glm::vec3 RigidBodyComponent::calcSupportPoint(const glm::vec3 &direction, const Transform &transform) const {
    const glm::mat3 modelNoTranslation = glm::mat3(transform.getModel());
    return transform.toWorld(calcSupportPoint(glm::transpose(modelNoTranslation) * direction));
}

const RigidBodyComponent::Info &RigidBodyComponent::getInfo() const {
    return m_Info;
}

const AABB &RigidBodyComponent::getAABB() const {
    return m_AABB;
}

} // namespace Physbuzz
