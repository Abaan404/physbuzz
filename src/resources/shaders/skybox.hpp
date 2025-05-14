#pragma once

#include "../../objects/skybox.hpp"
#include "../uniforms/camera.hpp"
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/resources/manager.hpp>
#include <string>

inline Physbuzz::ShaderPipelineResource shaderSkybox = {{
    .vertex = {.file = {.path = "resources/shaders/skybox/skybox.vert"}},
    .fragment = {.file = {.path = "resources/shaders/skybox/skybox.frag"}},
    .setup = [](Physbuzz::ShaderPipelineResource *pipeline) {
        Physbuzz::ResourceRegistry::get<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->bindPipeline(pipeline, 1);
    },
    .draw = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const SkyboxComponent &skybox = scene.getComponent<SkyboxComponent>(object);
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);

        const Physbuzz::CubemapResource *cubemap = Physbuzz::ResourceRegistry::get<Physbuzz::CubemapResource>(skybox.cubemap);
        PBZ_ASSERT(cubemap, std::format("[Renderer] CubemapResource '{}' unknown.", skybox.cubemap));

        const Physbuzz::ModelResource *model = Physbuzz::ResourceRegistry::get<Physbuzz::ModelResource>(render.model);
        PBZ_ASSERT(model, std::format("[Renderer] ModelResource '{}' unknown.", render.model));

        cubemap->bind(true);

        for (const Physbuzz::Mesh &mesh : model->getMeshs()) {
            mesh.bind();
            mesh.draw();
            mesh.unbind();
        }

        cubemap->unbind(true);
    },
}};
