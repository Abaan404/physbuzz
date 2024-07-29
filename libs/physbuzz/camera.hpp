#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

class Camera {
  public:
    Camera();
    ~Camera();

    void translate(const glm::vec3 &direction);
    void rotate(const glm::quat &quat);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
};

} // namespace Physbuzz
