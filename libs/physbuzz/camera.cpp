#include "camera.hpp"

#include "logging.hpp"
#include <glad/gl.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Physbuzz {

Camera::Camera() {}

Camera::~Camera() {}

void Camera::build() {
    glEnable(GL_DEPTH_TEST);
}

void Camera::destroy() {}

void Camera::resize(const glm::ivec2 &resolution, const Depth &depth) {
    switch (m_ProjectionType) {
    case ProjectionType::Prespective: {
        m_Prespective.aspect = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
        setPrespective(m_Prespective, depth);
    } break;

    case ProjectionType::Orthographic2D: {
        setOrthographic(resolution);
    } break;

    case ProjectionType::Orthographic:
    case ProjectionType::Unknown:
        break;
    }
}

void Camera::setPrespective(const Prespective &prespective, const Depth &depth) {
    m_ProjectionType = ProjectionType::Prespective;
    m_Projection = glm::perspective(prespective.fovy, prespective.aspect, depth.near, depth.far);
    m_Prespective = prespective;
    m_Depth = depth;
}

void Camera::setOrthographic(const Orthographic &orthographic, const Depth &depth) {
    m_ProjectionType = ProjectionType::Orthographic;
    m_Projection = glm::ortho(orthographic.left, orthographic.right, orthographic.bottom, orthographic.top, depth.near, depth.far);
    m_Orthographic = orthographic;
    m_Depth = depth;
}

void Camera::setOrthographic(const glm::ivec2 &resolution) {
    m_ProjectionType = ProjectionType::Orthographic2D;
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

void Camera::resetView() {
    m_Position = {0.0f, 0.0f, 0.0f};
    m_Orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    updateView();
}

void Camera::updateView() {
    const glm::mat4 rotation = glm::mat4(glm::conjugate(glm::quatLookAt(getFacing(), getUp())));
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_Position);

    m_View = rotation * translation;
}

void Camera::rotate(const glm::quat &delta) {
    if (m_ProjectionType == ProjectionType::Orthographic2D) {
        return;
    }

    m_Orientation = delta * m_Orientation;
    updateView();
}

void Camera::translate(const glm::vec3 &delta) {
    if (m_ProjectionType == ProjectionType::Orthographic2D) {
        return;
    }

    m_Position += delta;
    updateView();
}

void Camera::setView(const glm::mat4 &view) {
    m_View = view;
}

void Camera::setProjection(const glm::mat4 &projection) {
    m_Projection = projection;
    m_ProjectionType = ProjectionType::Unknown;
}

const glm::mat4 &Camera::getView() const {
    return m_View;
}

const glm::mat4 &Camera::getProjection() const {
    return m_Projection;
}

void Camera::setDepth(const Depth &depth) {
    m_Depth = depth;
}

const Camera::Depth &Camera::getDepth() const {
    return m_Depth;
}

const Camera::ProjectionType &Camera::getProjectionType() const {
    return m_ProjectionType;
}

void Camera::setPosition(const glm::vec3 &position) {
    m_Position = position;
    updateView();
}

const glm::vec3 &Camera::getPosition() const {
    return m_Position;
}

void Camera::setFacing(const glm::vec3 &facing) {
    glm::quat rotation = glm::rotation(getFacing(), glm::normalize(facing));
    rotate(rotation);
}

const glm::vec3 Camera::getFacing() const {
    return m_Orientation * glm::vec3(0.0f, 0.0f, -1.0f);
}

void Camera::setUp(const glm::vec3 &up) {
    glm::quat rotation = glm::rotation(getUp(), glm::normalize(up));
    rotate(rotation);
}

const glm::vec3 Camera::getUp() const {
    return m_Orientation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::setRight(const glm::vec3 &right) {
    glm::quat rotation = glm::rotation(getRight(), glm::normalize(right));
    rotate(rotation);
}

const glm::vec3 Camera::getRight() const {
    return glm::cross(getFacing(), getUp());
}

void Camera::setOrientation(const glm::quat &orientaton) {
    m_Orientation = orientaton;
    updateView();
}

const glm::quat &Camera::getOrientation() const {
    return m_Orientation;
}

const Camera::Prespective &Camera::getPrespective() const {
    if (m_ProjectionType != ProjectionType::Prespective) {
        Logger::WARNING("[Camera] Trying to fetch a non-prespective camera");
    }

    return m_Prespective;
}

const Camera::Orthographic &Camera::getOrthographic() const {
    if (m_ProjectionType != ProjectionType::Orthographic || m_ProjectionType != ProjectionType::Orthographic2D) {
        Logger::WARNING("[Camera] Trying to fetch a non-orthographic camera");
    }

    return m_Orthographic;
}

} // namespace Physbuzz
