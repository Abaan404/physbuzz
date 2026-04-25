#include "lights.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/geometric.hpp>

namespace Physbuzz {

DirectionalLightComponent::DirectionalLightComponent() {}

DirectionalLightComponent::DirectionalLightComponent(const Info &info) {
    update(info);
}

void DirectionalLightComponent::update(const Info &info) {
    m_Info = info;
    m_Info.direction = glm::normalize(m_Info.direction);
    updateProjectionView();
}

void DirectionalLightComponent::setDirection(const glm::vec3 &direction) {
    m_Info.direction = glm::normalize(direction);
    updateProjectionView();
}

void DirectionalLightComponent::setIntensity(const glm::vec3 &intensity) {
    m_Info.intensity = intensity;
}

const DirectionalLightComponent::Info &DirectionalLightComponent::getInfo() const {
    return m_Info;
}

const glm::mat4 &DirectionalLightComponent::getProjectionView() const {
    return m_ProjectionView;
}

const Frustum &DirectionalLightComponent::getFrustum() const {
    return m_Frustum;
}

void DirectionalLightComponent::updateProjectionView() {
    glm::mat4 projection = glm::ortho(-m_Info.orthoSize, m_Info.orthoSize, -m_Info.orthoSize, m_Info.orthoSize, 1.0f, m_Info.depth);
    glm::mat4 view = glm::lookAt(-m_Info.direction * m_Info.depth / 2.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});

    m_ProjectionView = projection * view;
    updateFrustum();
}

void DirectionalLightComponent::updateFrustum() {
    m_Frustum.update(m_ProjectionView);
}

PointLightComponent::PointLightComponent() {}

PointLightComponent::PointLightComponent(const Info &info) {
    update(info);
}

void PointLightComponent::setPosition(const glm::vec3 &position) {
    m_Info.position = position;
    updateProjectionViews();
}

void PointLightComponent::setIntensity(const glm::vec3 &intensity) {
    m_Info.intensity = intensity;
}

void PointLightComponent::update(const Info &info) {
    m_Info = info;
    updateProjectionViews();
}

const PointLightComponent::Info &PointLightComponent::getInfo() const {
    return m_Info;
}

const std::array<glm::mat4, 6> &PointLightComponent::getProjectionViews() const {
    return m_ProjectionView;
}

const std::array<Frustum, 6> &PointLightComponent::getFrustums() const {
    return m_Frustums;
}

void PointLightComponent::updateProjectionViews() {
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, m_Info.depth);

    m_ProjectionView = {
        projection * glm::lookAt(m_Info.position, m_Info.position + glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
        projection * glm::lookAt(m_Info.position, m_Info.position + glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
        projection * glm::lookAt(m_Info.position, m_Info.position + glm::vec3(0.0f, 1.0f, 0.0f), {0.0f, 0.0f, 1.0f}),
        projection * glm::lookAt(m_Info.position, m_Info.position + glm::vec3(0.0f, -1.0f, 0.0f), {0.0f, 0.0f, -1.0f}),
        projection * glm::lookAt(m_Info.position, m_Info.position + glm::vec3(0.0f, 0.0f, 1.0f), {0.0f, -1.0f, 0.0f}),
        projection * glm::lookAt(m_Info.position, m_Info.position + glm::vec3(0.0f, 0.0f, -1.0f), {0.0f, -1.0f, 0.0f}),
    };

    updateFrustums();
}

void PointLightComponent::updateFrustums() {
    for (std::size_t i = 0; i < 6; i++) {
        m_Frustums[i].update(m_ProjectionView[i]);
    }
}

SpotLightComponent::SpotLightComponent() {}

SpotLightComponent::SpotLightComponent(const Info &info) {
    update(info);
}

void SpotLightComponent::update(const Info &info) {
    m_Info = info;
    m_Info.direction = glm::normalize(m_Info.direction);

    updateProjectionView();
}

void SpotLightComponent::setPosition(const glm::vec3 &position) {
    m_Info.position = position;
    updateProjectionView();
}

void SpotLightComponent::setDirection(const glm::vec3 &direction) {
    m_Info.direction = glm::normalize(direction);
    updateProjectionView();
}

void SpotLightComponent::setIntensity(const glm::vec3 &intensity) {
    m_Info.intensity = intensity;
    updateProjectionView();
}

void SpotLightComponent::setCutoff(float inner, float outer) {
    m_Info.cutOff = glm::cos(inner);
    m_Info.outerCutOff = glm::cos(outer);
    updateProjectionView();
}

const SpotLightComponent::Info &SpotLightComponent::getInfo() const {
    return m_Info;
}

const glm::mat4 &SpotLightComponent::getProjectionView() const {
    return m_ProjectionView;
}

const Frustum &SpotLightComponent::getFrustum() const {
    return m_Frustum;
}

void SpotLightComponent::updateProjectionView() {
    const glm::mat4 view = glm::lookAt(m_Info.position, m_Info.position + m_Info.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 projection = glm::perspective(2.0f * glm::acos(m_Info.outerCutOff), 1.0f, 0.1f, m_Info.depth);

    m_ProjectionView = projection * view;
}

void SpotLightComponent::updateFrustum() {
    m_Frustum.update(m_ProjectionView);
}

} // namespace Physbuzz
