#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

struct PointLightComponent {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};

    glm::vec3 ambient = {1.0f, 1.0f, 1.0f};
    glm::vec3 diffuse = {1.0f, 1.0f, 1.0f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};

    float constant = 1.0f;
    float linear = 1.0f;
    float quadratic = 1.0f;
};

struct DirectionalLightComponent {
    glm::vec3 direction = {0.0f, 0.0f, 0.0f};

    glm::vec3 ambient = {1.0f, 1.0f, 1.0f};
    glm::vec3 diffuse = {1.0f, 1.0f, 1.0f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
};

struct SpotLightComponent {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 direction = {0.0f, 0.0f, 0.0f};

    glm::vec3 ambient = {1.0f, 1.0f, 1.0f};
    glm::vec3 diffuse = {1.0f, 1.0f, 1.0f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};

    float constant = 1.0f;
    float linear = 1.0f;
    float quadratic = 1.0f;

    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(17.5f));
};

} // namespace Physbuzz
