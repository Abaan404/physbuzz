#pragma once

#include <glm/glm.hpp>

struct UniformCamera {
    glm::vec3 position;
    float _padding0;
    glm::mat4x4 view;
    glm::mat4x4 projection;
};
