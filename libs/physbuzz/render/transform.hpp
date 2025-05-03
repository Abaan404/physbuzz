#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physbuzz {

struct TransformComponent {
    void update();
    void reset();

    const glm::vec3 toWorld(const glm::vec3 &local) const;
    const glm::vec3 toLocal(const glm::vec3 &world) const;

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 matrix = glm::mat4(1.0f);
};

} // namespace Physbuzz
