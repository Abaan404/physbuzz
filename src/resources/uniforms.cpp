#include "builder.hpp"

#include "uniforms/camera.hpp"
#include "uniforms/time.hpp"
#include "uniforms/window.hpp"
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/resources/resources.hpp>

void ResourceBuilder::buildUniforms() {
    Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformCamera>>::insert("camera", {});
    Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformTime>>::insert("time", {});
    Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformWindow>>::insert("window", {});
}

void ResourceBuilder::destroyUniforms() {
    Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformCamera>>::erase("camera");
    Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformTime>>::erase("time");
    Physbuzz::ResourceRegistry<Physbuzz::UniformBuffer<UniformWindow>>::erase("window");
}
