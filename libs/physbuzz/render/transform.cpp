#include "transform.hpp"

namespace Physbuzz {

const glm::vec3 TransformComponent::toWorld(const glm::vec3 &local) const {
    return matrix * glm::vec4(local, 1.0f);
}

const glm::vec3 TransformComponent::toLocal(const glm::vec3 &world) const {
    return glm::inverse(matrix) * glm::vec4(world, 1.0f);
}

void TransformComponent::reset() {
    position = {0.0f, 0.0f, 0.0f};
    scale = {1.0f, 1.0f, 1.0f};
    orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
}

void TransformComponent::update() {
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
    const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::angle(orientation), glm::axis(orientation)); // conjugate?
    const glm::mat4 stretch = glm::scale(glm::mat4(1.0f), scale);

    matrix = translation * rotation * stretch;
}

} // namespace Physbuzz
