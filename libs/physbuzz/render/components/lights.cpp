#include "lights.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>

namespace Physbuzz {

DirectionalLightComponent::DirectionalLightComponent(const Info &info) {
    update(info);
}

void DirectionalLightComponent::setDirection(const glm::vec3 &direction) {
    m_Info.direction = glm::normalize(direction);
    updateProjectionView();
}

void DirectionalLightComponent::setIntensity(const glm::vec3 &intensity) {
    m_Info.intensity = intensity;
}

void DirectionalLightComponent::update(const Info &info) {
    m_Info = info;
    m_Info.direction = glm::normalize(m_Info.direction);
    updateProjectionView();
}

void DirectionalLightComponent::updateProjectionView() {
    glm::mat4 projection = glm::ortho(-m_Info.orthoSize, m_Info.orthoSize, -m_Info.orthoSize, m_Info.orthoSize, 1.0f, m_Info.depth);
    glm::mat4 view = glm::lookAt(-m_Info.direction * m_Info.depth / 2.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});

    m_ProjectionView = projection * view;
}

const DirectionalLightComponent::Info &DirectionalLightComponent::getInfo() const {
    return m_Info;
}

const glm::mat4 &DirectionalLightComponent::getProjectionView() const {
    return m_ProjectionView;
}

PointLightComponent::PointLightComponent(const Info &info) {
    update(info);
}

void PointLightComponent::setPosition(const glm::vec3 &position) {
    m_Info.position = position;
}

void PointLightComponent::setIntensity(const glm::vec3 &intensity) {
    m_Info.intensity = intensity;
}

void PointLightComponent::update(const Info &info) {
    m_Info = info;
}

const PointLightComponent::Info &PointLightComponent::getInfo() const {
    return m_Info;
}

SpotLightComponent::SpotLightComponent(const Info &info) {
    update(info);
}

void SpotLightComponent::setPosition(const glm::vec3 &position) {
    m_Info.position = position;
}

void SpotLightComponent::setDirection(const glm::vec3 &direction) {
    m_Info.direction = glm::normalize(direction);
}

void SpotLightComponent::setIntensity(const glm::vec3 &intensity) {
    m_Info.intensity = intensity;
}

void SpotLightComponent::setCutoff(float inner, float outer) {
    m_Info.cutOff = glm::cos(inner);
    m_Info.outerCutOff = glm::cos(outer);
}

void SpotLightComponent::update(const Info &info) {
    m_Info = info;
    m_Info.direction = glm::normalize(m_Info.direction);
}

const SpotLightComponent::Info &SpotLightComponent::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
