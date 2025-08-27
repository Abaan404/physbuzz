#include "builder.hpp"

#include "shaders/circle.hpp"
#include "shaders/debugnormal.hpp"
#include "shaders/gamma.hpp"
#include "shaders/skybox.hpp"

void ResourceBuilder::buildShaders() {
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::insert("circle", std::move(shaderCircle));
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::insert("skybox", std::move(shaderSkybox));
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::insert("debug/normal", std::move(shaderDebugNormal));
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::insert("gamma", std::move(shaderGamma));

    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::watch();
}

void ResourceBuilder::destroyShaders() {
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::erase("circle");
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::erase("skybox");
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::erase("debug/normal");
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::erase("gamma");
}
