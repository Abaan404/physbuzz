#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

struct PointLightComponent {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

} // namespace Physbuzz
