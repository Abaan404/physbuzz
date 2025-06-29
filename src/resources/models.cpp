#include "builder.hpp"

#include <physbuzz/render/model.hpp>

void ResourceBuilder::buildModels() {
    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        "backpack",
        {{
            "resources/models/backpack/backpack.obj",
        }});
}

void ResourceBuilder::destroyModels() {
    Physbuzz::ResourceRegistry<Physbuzz::Model>::erase("backpack");
}
