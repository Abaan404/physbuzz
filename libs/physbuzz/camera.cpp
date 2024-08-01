#include "camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Physbuzz {

Camera::Camera() {}

Camera::~Camera() {}

void Camera::build() {
    updateView();
}

void Camera::destroy() {}

void Camera::resize(const glm::ivec2 &resolution) {
    switch (m_Mode) {
    case Mode::Prespective: {
        m_Prespective.aspect = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
        setPrespective(m_Prespective, m_Depth);
    } break;

    case Mode::Orthographic2D: {
        m_Orthographic.right = static_cast<float>(resolution.x);
        m_Orthographic.bottom = static_cast<float>(resolution.y);
        setOrthographic(m_Orthographic, m_Depth);
    } break;

    case Mode::Orthographic:
    case Mode::Unknown:
        break;
    }
}

void Camera::setPrespective(const Prespective &prespective, const Depth &depth) {
    m_Mode = Mode::Prespective;
    m_Projection = glm::perspective(prespective.fovy, prespective.aspect, depth.near, depth.far);
    m_Prespective = prespective;
    m_Depth = depth;
}

void Camera::setOrthographic(const Orthographic &orthographic, const Depth &depth) {
    m_Mode = Mode::Orthographic;
    m_Projection = glm::ortho(orthographic.left, orthographic.right, orthographic.bottom, orthographic.top, depth.near, depth.far);
    m_Orthographic = orthographic;
    m_Depth = depth;
}

void Camera::setOrthographic(const glm::ivec2 &resolution) {
    m_Mode = Mode::Orthographic2D;
    m_Orthographic = {
        .left = 0.0f,
        .right = static_cast<float>(resolution.x),
        .bottom = static_cast<float>(resolution.y),
        .top = 0.0f,
    };

    m_Depth = {
        .near = -1.0f,
        .far = 1.0f,
    };

    m_Projection = glm::ortho(m_Orthographic.left, m_Orthographic.right, m_Orthographic.bottom, m_Orthographic.top, m_Depth.near, m_Depth.far);
}

void Camera::updateView() {
    m_View = glm::lookAt(m_Position, m_Position + m_Facing, m_Up);
}

void Camera::setView(const glm::mat4 &view) {
    m_View = view;
}

void Camera::setProjection(const glm::mat4 &transform) {
    m_Projection = transform;
}

const glm::mat4 &Camera::getView() const {
    return m_View;
}

const glm::mat4 &Camera::getProjection() const {
    return m_Projection;
}

const Camera::Mode &Camera::getMode() const {
    return m_Mode;
}

void Camera::setPosition(const glm::vec3 &position) {
    m_Position = position;
    updateView();
}

const glm::vec3 &Camera::getPosition() const {
    return m_Position;
}

void Camera::setFacing(const glm::vec3 &facing) {
    m_Facing = facing;
    updateView();
}

const glm::vec3 &Camera::getFacing() const {
    return m_Facing;
}

void Camera::setUp(const glm::vec3 &up) {
    m_Up = up;
    updateView();
}

const glm::vec3 &Camera::getUp() const {
    return m_Up;
}

} // namespace Physbuzz
