#include "builder.hpp"

#include "uniforms/camera.hpp"
#include "uniforms/time.hpp"
#include "uniforms/window.hpp"
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/resources/manager.hpp>

void ResourceBuilder::buildUniforms() {
    Physbuzz::ResourceRegistry<Physbuzz::UniformBufferResource<UniformCamera>>::insert("camera", {});
    Physbuzz::ResourceRegistry<Physbuzz::UniformBufferResource<UniformTime>>::insert("time", {});
    Physbuzz::ResourceRegistry<Physbuzz::UniformBufferResource<UniformWindow>>::insert("window", {});
}

void ResourceBuilder::destroyUniforms() {
    Physbuzz::ResourceRegistry<Physbuzz::UniformBufferResource<UniformCamera>>::erase("camera");
    Physbuzz::ResourceRegistry<Physbuzz::UniformBufferResource<UniformTime>>::erase("time");
    Physbuzz::ResourceRegistry<Physbuzz::UniformBufferResource<UniformWindow>>::erase("window");
}
