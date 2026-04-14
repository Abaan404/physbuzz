#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Physbuzz {

CameraFrustum::CameraFrustum(const Info &info)
    : m_Info(info) {}

void CameraFrustum::update(const Info &info) {
    m_Info = info;
}

void CameraFrustum::update(const CameraComponent &camera) {
    update(camera.getProjection() * camera.getView());
}

void CameraFrustum::update(const glm::mat4 &projectionView) {
    // https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
    const glm::mat4 viewProjection = glm::transpose(projectionView);

    glm::vec4 left = viewProjection[3] + viewProjection[0];
    glm::vec4 right = viewProjection[3] - viewProjection[0];
    glm::vec4 bottom = viewProjection[3] + viewProjection[1];
    glm::vec4 top = viewProjection[3] - viewProjection[1];
    glm::vec4 near = viewProjection[3] + viewProjection[2];
    glm::vec4 far = viewProjection[3] - viewProjection[2];

    m_Info = {
        .left = {{glm::normalize(glm::vec3(left)), left.w / glm::length(glm::vec3(left))}},
        .right = {{glm::normalize(glm::vec3(right)), right.w / glm::length(glm::vec3(right))}},
        .bottom = {{glm::normalize(glm::vec3(bottom)), bottom.w / glm::length(glm::vec3(bottom))}},
        .top = {{glm::normalize(glm::vec3(top)), top.w / glm::length(glm::vec3(top))}},
        .near = {{glm::normalize(glm::vec3(near)), near.w / glm::length(glm::vec3(near))}},
        .far = {{glm::normalize(glm::vec3(far)), far.w / glm::length(glm::vec3(far))}},
    };
}

const CameraFrustum::Info &CameraFrustum::getInfo() const {
    return m_Info;
}

CameraComponent::CameraComponent(const Info &info)
    : m_Info(info) {
    update(info);
}

void CameraComponent::resize(const glm::ivec2 &resolution) {
    m_Info.resolution = resolution;

    if (m_Info.projection == Projection::Perspective && resolution.y != 0) {
        m_Info.perspective.aspect = static_cast<float>(resolution.x) / resolution.y;
    }

    updateProjection();
}

void CameraComponent::reset() {
    m_Info.view.position = glm::vec3(0.0f);
    m_Info.view.orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    updateView();
}

void CameraComponent::setProjection(Projection projection) {
    m_Info.projection = projection;
    updateProjection();
}

void CameraComponent::update(const Info &info) {
    m_Info = info;

    if (m_Info.projection == Projection::Perspective && m_Info.resolution.y != 0) {
        m_Info.perspective.aspect = static_cast<float>(m_Info.resolution.x) / m_Info.resolution.y;
    }

    updateView();
    updateProjection();
}

void CameraComponent::setPosition(const glm::vec3 &position) {
    m_Info.view.position = position;
    updateView();
}

void CameraComponent::setOrientation(const glm::quat &orientation) {
    m_Info.view.orientation = orientation;
    updateView();
}

void CameraComponent::setFacing(const glm::vec3 &facing) {
    m_Info.view.orientation = glm::rotation(getFacing(), glm::normalize(facing)) * m_Info.view.orientation;
    updateView();
}

glm::vec3 CameraComponent::getFacing() const {
    return m_Info.view.orientation * glm::vec3(0.0f, 0.0f, -1.0f);
}

void CameraComponent::setUp(const glm::vec3 &up) {
    m_Info.view.orientation = glm::rotation(getUp(), glm::normalize(up)) * m_Info.view.orientation;
    updateView();
}

glm::vec3 CameraComponent::getUp() const {
    return m_Info.view.orientation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void CameraComponent::setRight(const glm::vec3 &right) {
    m_Info.view.orientation = glm::rotation(getRight(), glm::normalize(right)) * m_Info.view.orientation;
    updateView();
}

glm::vec3 CameraComponent::getRight() const {
    return m_Info.view.orientation * glm::vec3(1.0f, 0.0f, 0.0f);
}

const glm::mat4 &CameraComponent::getProjection() const {
    return m_Projection;
}

const glm::mat4 &CameraComponent::getView() const {
    return m_View;
}

const CameraFrustum &CameraComponent::getFrustum() const {
    return m_Frustum;
}

const CameraComponent::Info &CameraComponent::getInfo() const {
    return m_Info;
}

void CameraComponent::updateProjection() {
    switch (m_Info.projection) {
    case Projection::Perspective:
        m_Projection = glm::perspective(
            m_Info.perspective.fovy,
            m_Info.perspective.aspect,
            m_Info.depth.near,
            m_Info.depth.far);
        break;

    case Projection::Orthographic:
        m_Projection = glm::ortho(
            m_Info.orthographic.left,
            m_Info.orthographic.right,
            m_Info.orthographic.bottom,
            m_Info.orthographic.top,
            m_Info.depth.near,
            m_Info.depth.far);
        break;

    default:
        m_Projection = glm::mat4(1.0f);
        break;
    }

    m_Projection[1][1] *= -1;

    updateFrustum();
}

void CameraComponent::updateView() {
    const glm::mat4 rotation = glm::mat4_cast(glm::conjugate(m_Info.view.orientation));
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_Info.view.position);

    m_View = rotation * translation;

    updateFrustum();
}

void CameraComponent::updateFrustum() {
    m_Frustum.update(*this);
}

} // namespace Physbuzz
