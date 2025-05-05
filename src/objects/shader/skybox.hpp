#pragma once

#include "../common.hpp"
#include "../skybox.hpp"
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/resources/manager.hpp>
#include <string>

const ShaderComponent s_SkyboxShader = {
    .resource = "skybox",
    .render = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const SkyboxComponent &skybox = scene.getComponent<SkyboxComponent>(object);
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);

        const Physbuzz::CubemapResource *cubemap = Physbuzz::ResourceRegistry::get<Physbuzz::CubemapResource>(skybox.cubemap);
        PBZ_ASSERT(cubemap, std::format("[Renderer] CubemapResource '{}' unknown.", skybox.cubemap));

        const Physbuzz::ModelResource *model = Physbuzz::ResourceRegistry::get<Physbuzz::ModelResource>(render.model);
        PBZ_ASSERT(model, std::format("[Renderer] ModelResource '{}' unknown.", render.model));

        const Physbuzz::ShaderPipelineResource *pipeline = Physbuzz::ResourceRegistry::get<Physbuzz::ShaderPipelineResource>("skybox");
        PBZ_ASSERT(pipeline, std::format("[Renderer] ShaderPipelineResource '{}' unknown.", "default"));

        cubemap->bind(true);

        for (const Physbuzz::Mesh &mesh : model->getMeshs()) {
            mesh.bind();
            mesh.draw();
            mesh.unbind();
        }

        cubemap->unbind(true);
    },
};
