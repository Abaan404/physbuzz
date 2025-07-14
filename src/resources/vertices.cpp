#include "builder.hpp"

#include "vertex/default.hpp"
#include <physbuzz/render/mesh.hpp>

void ResourceBuilder::buildVertices() {
    Physbuzz::ResourceRegistry<Physbuzz::VertexAttribute>::insert(
        "default",
        {{
            .attributes = {
                {
                    .type = Physbuzz::Types::Float,
                    .size = 3,
                    .offset = offsetof(VertexDefault, position),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = 3,
                    .offset = offsetof(VertexDefault, normal),
                },
                {
                    .type = Physbuzz::Types::Float,
                    .size = 2,
                    .offset = offsetof(VertexDefault, texCoords),
                },
            },
            .size = sizeof(VertexDefault),
        }});
}

void ResourceBuilder::destroyVertices() {
    Physbuzz::ResourceRegistry<Physbuzz::VertexAttribute>::erase("default");
}
