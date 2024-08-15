#include "camera.hpp"

#include "../debug/logging.hpp"
#include <glad/gl.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Physbuzz {

void View::reset() {
    position = {0.0f, 0.0f, 0.0f};
    orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    update();
}

void View::update() {
    const glm::mat4 rotation = glm::mat4(glm::conjugate(glm::quatLookAt(getFacing(), getUp())));
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), -position);

    matrix = rotation * translation;
}

void View::setFacing(const glm::vec3 &facing) {
    orientation = glm::rotation(getFacing(), glm::normalize(facing)) * orientation;
}

const glm::vec3 View::getFacing() const {
    return orientation * glm::vec3(0.0f, 0.0f, -1.0f);
}

void View::setUp(const glm::vec3 &up) {
    orientation = glm::rotation(getUp(), glm::normalize(up)) * orientation;
}

const glm::vec3 View::getUp() const {
    return orientation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void View::setRight(const glm::vec3 &right) {
    orientation = glm::rotation(getRight(), glm::normalize(right)) * orientation;
}

const glm::vec3 View::getRight() const {
    return glm::cross(getFacing(), getUp());
}

Camera::Camera() {}

Camera::~Camera() {}

void Camera::resize(const glm::ivec2 &resolution, const Depth &depth) {
    switch (m_ProjectionType) {
    case Type::Prespective: {
        m_Prespective.aspect = static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
        setPrespective(m_Prespective, depth);
    } break;

    case Type::Orthographic2D: {
        setOrthographic(resolution);
    } break;

    case Type::Orthographic:
    case Type::Unknown:
        break;
    }
}

void Camera::setPrespective(const Prespective &prespective, const Depth &depth) {
    glEnable(GL_DEPTH_TEST);

    m_ProjectionType = Type::Prespective;
    m_Projection = glm::perspective(prespective.fovy, prespective.aspect, depth.near, depth.far);
    m_Prespective = prespective;
    m_Depth = depth;
}

void Camera::setOrthographic(const Orthographic &orthographic, const Depth &depth) {
    glEnable(GL_DEPTH_TEST);

    m_ProjectionType = Type::Orthographic;
    m_Projection = glm::ortho(orthographic.left, orthographic.right, orthographic.bottom, orthographic.top, depth.near, depth.far);
    m_Orthographic = orthographic;
    m_Depth = depth;
}

void Camera::setOrthographic(const glm::ivec2 &resolution) {
    glDisable(GL_DEPTH_TEST);

    m_ProjectionType = Type::Orthographic2D;
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

void Camera::setProjection(const glm::mat4 &projection) {
    m_Projection = projection;
    m_ProjectionType = Type::Unknown;
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

const Camera::Type &Camera::getProjectionType() const {
    return m_ProjectionType;
}

const Camera::Prespective &Camera::getPrespective() const {
    if (m_ProjectionType != Type::Prespective) {
        Logger::WARNING("[Camera] Trying to fetch a non-prespective camera");
    }

    return m_Prespective;
}

const Camera::Orthographic &Camera::getOrthographic() const {
    if (m_ProjectionType != Type::Orthographic && m_ProjectionType != Type::Orthographic2D) {
        Logger::WARNING("[Camera] Trying to fetch a non-orthographic camera");
    }

    return m_Orthographic;
}

} // namespace Physbuzz
