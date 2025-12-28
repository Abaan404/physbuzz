#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

struct PointLightComponent {
    alignas(16) glm::vec3 position = {0.0f, 0.0f, 0.0f};
    alignas(16) glm::vec3 intensity = {0.0f, 0.0f, 0.0f};
};

struct DirectionalLightComponent {
    alignas(16) glm::vec3 direction = {0.0f, 0.0f, 0.0f};
    alignas(16) glm::vec3 intensity = {0.0f, 0.0f, 0.0f};
};

struct SpotLightComponent {
    alignas(16) glm::vec3 position = {0.0f, 0.0f, 0.0f};
    alignas(16) glm::vec3 direction = {0.0f, 0.0f, 0.0f};
    alignas(16) glm::vec3 intensity = {0.0f, 0.0f, 0.0f};

    float cutOff = glm::radians(12.5f);
    float outerCutOff = glm::radians(17.5f);
};

} // namespace Physbuzz
