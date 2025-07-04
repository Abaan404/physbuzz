#pragma once

#include "../../render/mesh.hpp"

namespace Physbuzz {

namespace Builtin {

namespace VertexDefault {

struct Format {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 texCoords;
};

inline Resource<VertexAttribute> Resource = {"builtin/default"};

bool build();

} // namespace ScreenQuad

} // namespace Builtin

} // namespace Physbuzz
