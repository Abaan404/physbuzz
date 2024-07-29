#include "camera.hpp"

namespace Physbuzz {

Camera::Camera() {}

Camera::~Camera() {}

void Camera::rotate(const glm::quat &quat) {
    // model = glm::rotate(model, glm::angle(quat), glm::axis(quat));
}

void Camera::translate(const glm::vec3 &direction) {
    // glm::translate(view, -direction);
}

} // namespace Physbuzz
