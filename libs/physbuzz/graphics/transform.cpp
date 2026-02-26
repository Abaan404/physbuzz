#include "transform.hpp"

namespace Physbuzz {

Transform::Transform(const Info &info)
    : m_Info(info) {
    update(info);
}

const glm::vec3 Transform::toWorld(const glm::vec3 &local) const {
    return m_Model * glm::vec4(local, 1.0f);
}

const glm::vec3 Transform::toLocal(const glm::vec3 &world) const {
    return glm::inverse(m_Model) * glm::vec4(world, 1.0f);
}

void Transform::setPosition(const glm::vec3 position) {
    m_Info.position = position;
    updateModel();
}

void Transform::setScale(const glm::vec3 scale) {
    m_Info.scale = scale;
    updateModel();
}

void Transform::setOrientation(const glm::quat orientation) {
    m_Info.orientation = orientation;
    updateModel();
}

void Transform::setOrientation(float magnitude, const glm::vec3 &direction) {
    m_Info.orientation = glm::angleAxis(magnitude, direction);
    updateModel();
}

void Transform::update(const Info &info) {
    m_Info = info;
    updateModel();
}

const Transform::Info &Transform::getInfo() const {
    return m_Info;
}

const glm::mat4 &Transform::getModel() const {
    return m_Model;
}
void Transform::updateModel() {
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_Info.position);
    const glm::mat4 rotation = glm::mat4_cast(m_Info.orientation);
    const glm::mat4 stretch = glm::scale(glm::mat4(1.0f), m_Info.scale);

    m_Model = translation * rotation * stretch;
}

} // namespace Physbuzz
