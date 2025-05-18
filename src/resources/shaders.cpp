#include "builder.hpp"

#include "shaders/circle.hpp"
#include "shaders/cube.hpp"
#include "shaders/default.hpp"
#include "shaders/quad.hpp"
#include "shaders/skybox.hpp"
#include "uniforms/camera.hpp"
#include "uniforms/time.hpp"
#include "uniforms/window.hpp"

static Physbuzz::EventID eventBuild;

void ResourceBuilder::buildShaders() {
    eventBuild = Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::Events.addCallback<Physbuzz::OnResourceBuild>([](const Physbuzz::OnResourceBuild &event) {
        if (event.identifier == "skybox") {
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->bindPipeline(1);
        }

        else if (event.identifier == "default") {
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->bindPipeline(1);
        }

        else if (event.identifier == "circle") {
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->bindPipeline(1);
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformWindow>>("window")->bindPipeline(2);
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformTime>>("time")->bindPipeline(3);
        }

        else if (event.identifier == "quad") {
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->bindPipeline(1);
        }

        else if (event.identifier == "cube") {
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->bindPipeline(1);
        }
    });

    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::insert("default", std::move(shaderDefault));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::insert("circle", std::move(shaderCircle));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::insert("quad", std::move(shaderQuad));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::insert("cube", std::move(shaderCube));
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::insert("skybox", std::move(shaderSkybox));

    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::watch();
}

void ResourceBuilder::destroyShaders() {
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::erase("default");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::erase("circle");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::erase("quad");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::erase("cube");
    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::erase("skybox");

    Physbuzz::ResourceRegistry<Physbuzz::ShaderPipelineResource>::Events.eraseCallback<Physbuzz::OnResourceBuild>(eventBuild);
}
