#pragma once

#include "../../render/mesh.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

namespace Builtin {

namespace VertexDefault {

struct Format {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoords;
};

inline Resource<VertexAttribute> Resource = {"builtin/default"};

bool build();

} // namespace VertexDefault

namespace VertexScreenQuad {

struct Format {
    glm::vec3 position;
    glm::vec2 texCoords;
};

inline Resource<VertexAttribute> Resource = {"builtin/screenquad"};

bool build();

} // namespace VertexScreenQuad

} // namespace Builtin

} // namespace Physbuzz
