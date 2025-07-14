#include "vertices.hpp"

namespace Physbuzz {

namespace Builtin {

bool VertexDefault::build() {
    if (ResourceRegistry<VertexAttribute>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<VertexAttribute>::insert(
        Resource.getIdentifier(),
        {{
            .attributes = {
                {
                    .type = Physbuzz::Types::Float,
                    .size = 3,
                    .offset = offsetof(Format, position),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = 3,
                    .offset = offsetof(Format, normal),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = 3,
                    .offset = offsetof(Format, tangent),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = 3,
                    .offset = offsetof(Format, bitangent),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = 2,
                    .offset = offsetof(Format, texCoords),
                },
            },
            .size = sizeof(Format),
        }});
}

bool VertexScreenQuad::build() {
    if (ResourceRegistry<VertexAttribute>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<VertexAttribute>::insert(
        Resource.getIdentifier(),
        {{
            .attributes = {
                {
                    .type = Physbuzz::Types::Float,
                    .size = 3,
                    .offset = offsetof(Format, position),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = 2,
                    .offset = offsetof(Format, texCoords),
                },
            },
            .size = sizeof(Format),
        }});
}

} // namespace Builtin

} // namespace Physbuzz
