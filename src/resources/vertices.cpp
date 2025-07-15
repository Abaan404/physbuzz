#include "builder.hpp"

#include "vertex/skybox.hpp"
#include <physbuzz/render/mesh.hpp>

void ResourceBuilder::buildVertices() {
    Physbuzz::ResourceRegistry<Physbuzz::VertexAttribute>::insert(
        "skybox",
        {{
            .attributes = {
                {
                    .type = Physbuzz::Types::Float,
                    .size = sizeof(VertexSkybox::position) / sizeof(decltype(VertexSkybox::position)::value_type),
                    .offset = offsetof(VertexSkybox, position),
                },
            },
            .size = sizeof(VertexSkybox),
        }});
}

void ResourceBuilder::destroyVertices() {
    Physbuzz::ResourceRegistry<Physbuzz::VertexAttribute>::erase("skybox");
}
