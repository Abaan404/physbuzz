#include "builder.hpp"

#include <physbuzz/render/model.hpp>
#include <physbuzz/resources/manager.hpp>

void ResourceBuilder::buildModels() {
    Physbuzz::ResourceRegistry<Physbuzz::ModelResource>::insert(
        "backpack",
        {{
            "resources/models/backpack/backpack.obj",
        }});
}

void ResourceBuilder::destroyModels() {
    Physbuzz::ResourceRegistry<Physbuzz::ModelResource>::erase("backpack");
}
