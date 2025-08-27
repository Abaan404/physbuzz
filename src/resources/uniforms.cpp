#include "builder.hpp"

#include "uniforms/time.hpp"
#include "uniforms/window.hpp"
#include <physbuzz/resources/resources.hpp>

void ResourceBuilder::buildUniforms() {
    // Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformWindow>>::insert("window", {});
    // Physbuzz::Resource<Physbuzz::UniformBuffer<UniformWindow>>("window")->bindPipeline(1);
    //
    // Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformTime>>::insert("time", {});
    // Physbuzz::Resource<Physbuzz::UniformBuffer<UniformTime>>("time")->bindPipeline(2);
}

void ResourceBuilder::destroyUniforms() {
//     Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformTime>>::erase("time");
//     Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformWindow>>::erase("window");
}
