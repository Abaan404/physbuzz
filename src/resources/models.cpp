#include "builder.hpp"

#include <physbuzz/graphics/model.hpp>

void ResourceBuilder::buildModels() {
    // Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
    //     "backpack",
    //     {{
    //         .path = "resources/models/backpack/backpack.obj",
    //         .meshes = {},
    //         .textures = {},
    //     }});
}

void ResourceBuilder::destroyModels() {
    // Physbuzz::ResourceRegistry<Physbuzz::Model>::erase("backpack");
}
