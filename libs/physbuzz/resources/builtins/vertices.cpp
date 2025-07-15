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
                    .size = sizeof(Format::position) / sizeof(decltype(Format::position)::value_type),
                    .offset = offsetof(Format, position),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = sizeof(Format::normal) / sizeof(decltype(Format::normal)::value_type),
                    .offset = offsetof(Format, normal),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = sizeof(Format::tangent) / sizeof(decltype(Format::tangent)::value_type),
                    .offset = offsetof(Format, tangent),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = sizeof(Format::texCoords) / sizeof(decltype(Format::texCoords)::value_type),
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
                    .size = sizeof(Format::position) / sizeof(decltype(Format::position)::value_type),
                    .offset = offsetof(Format, position),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = sizeof(Format::texCoords) / sizeof(decltype(Format::texCoords)::value_type),
                    .offset = offsetof(Format, texCoords),
                },
            },
            .size = sizeof(Format),
        }});
}

} // namespace Builtin

} // namespace Physbuzz
