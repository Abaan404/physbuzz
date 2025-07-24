#include "builder.hpp"

#include "shaders/circle.hpp"
#include "shaders/cube.hpp"
#include "shaders/debugnormal.hpp"
#include "shaders/default.hpp"
#include "shaders/gamma.hpp"
#include "shaders/quad.hpp"
#include "shaders/skybox.hpp"
#include <physbuzz/render/uniforms.hpp>

void ResourceBuilder::buildShaders() {
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert("default", std::move(shaderDefault));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert("circle", std::move(shaderCircle));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert("quad", std::move(shaderQuad));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert("cube", std::move(shaderCube));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert("skybox", std::move(shaderSkybox));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert("debug/normal", std::move(shaderDebugNormal));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::insert("gamma", std::move(shaderGamma));

    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::watch();
}

void ResourceBuilder::destroyShaders() {
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("default");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("circle");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("quad");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("cube");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("skybox");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("debug/normal");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipeline>::erase("gamma");
}
