#pragma once

#include "../common.hpp"
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/resources/manager.hpp>
#include <string>

const ShaderComponent s_CircleShader = {
    .resource = "circle",
    .render = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);
        const Physbuzz::ModelResource *model = Physbuzz::ResourceRegistry::get<Physbuzz::ModelResource>(render.model);
        const Physbuzz::ShaderPipelineResource *pipeline = Physbuzz::ResourceRegistry::get<Physbuzz::ShaderPipelineResource>("circle");

        // draw meshes
        for (const Physbuzz::Mesh &mesh : model->getMeshs()) {
            mesh.bind();
            mesh.draw();
            mesh.unbind();
        }
    },
};
