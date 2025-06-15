#include "camera.hpp"

#include <glm/gtx/quaternion.hpp>

namespace Physbuzz {

CameraComponent::CameraComponent(const CameraInfo &info)
    : m_Info(info) {
    resize(info.resolution);
}

void CameraComponent::resize(const glm::ivec2 &resolution) {
    switch (m_Info.type) {
    case CameraInfo::Type::Prespective:
        m_Info.prespective.aspect = resolution.y != 0 ? static_cast<float>(resolution.x) / resolution.y : 1.0f;
        setPrespective(m_Info.prespective);
        break;

    case CameraInfo::Type::Orthographic2D:
        setOrthographic2D(resolution);
        break;

    default:
        break;
    }
}

void CameraComponent::setOrthographic2D(const glm::ivec2 &resolution) {
    m_Info.type = CameraInfo::Type::Orthographic2D;
    m_Info.orthographic = {
        .left = 0.0f,
        .right = static_cast<float>(resolution.x),
        .bottom = static_cast<float>(resolution.y),
        .top = 0.0f,
    };

    m_Info.depth = {
        .near = -1.0f,
        .far = 1.0f,
    };

    m_Projection = glm::ortho(m_Info.orthographic.left, m_Info.orthographic.right, m_Info.orthographic.bottom, m_Info.orthographic.top, m_Info.depth.near, m_Info.depth.far);
}

void CameraComponent::setOrthographic(const CameraInfo::Orthographic &orthographic) {
    m_Info.type = CameraInfo::Type::Orthographic;
    m_Info.orthographic = orthographic;

    m_Projection = glm::ortho(orthographic.left, orthographic.right, orthographic.bottom, orthographic.top, m_Info.depth.near, m_Info.depth.far);
}

void CameraComponent::setPrespective(const CameraInfo::Prespective &prespective) {
    m_Info.type = CameraInfo::Type::Prespective;
    m_Info.prespective = prespective;

    m_Projection = glm::perspective(prespective.fovy, prespective.aspect, m_Info.depth.near, m_Info.depth.far);
}

void CameraComponent::setDepth(const CameraInfo::Depth &depth) {
    m_Info.depth = depth;

    switch (m_Info.type) {
    case CameraInfo::Type::Prespective:
        setPrespective(m_Info.prespective);
        break;

    case CameraInfo::Type::Orthographic:
        break;

    default:
        break;
    }
}

const glm::mat4 &CameraComponent::getProjection() const {
    return m_Projection;
}

void CameraComponent::reset() {
    m_Info.view.position = {0.0f, 0.0f, 0.0f};
    m_Info.view.orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    update();
}

void CameraComponent::setFacing(const glm::vec3 &facing) {
    m_Info.view.orientation = glm::rotation(getFacing(), glm::normalize(facing)) * m_Info.view.orientation;
    update();
}

const glm::vec3 CameraComponent::getFacing() const {
    return m_Info.view.orientation * glm::vec3(0.0f, 0.0f, -1.0f);
}

void CameraComponent::setUp(const glm::vec3 &up) {
    m_Info.view.orientation = glm::rotation(getUp(), glm::normalize(up)) * m_Info.view.orientation;
    update();
}

const glm::vec3 CameraComponent::getUp() const {
    return m_Info.view.orientation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void CameraComponent::setRight(const glm::vec3 &right) {
    m_Info.view.orientation = glm::rotation(getRight(), glm::normalize(right)) * m_Info.view.orientation;
    update();
}

const glm::vec3 CameraComponent::getRight() const {
    return m_Info.view.orientation * glm::vec3(1.0f, 0.0f, 0.0f);
}

void CameraComponent::setPosition(const glm::vec3 &position) {
    m_Info.view.position = position;
    update();
}

void CameraComponent::setOrientation(const glm::quat &orientation) {
    m_Info.view.orientation = orientation;
    update();
}

const CameraInfo &CameraComponent::getInfo() const {
    return m_Info;
}

const glm::mat4 &CameraComponent::getView() const {
    return m_View;
}

void CameraComponent::update() {
    const glm::mat4 rotation = glm::mat4_cast(glm::conjugate(m_Info.view.orientation));
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_Info.view.position);

    m_View = rotation * translation;
}

} // namespace Physbuzz
