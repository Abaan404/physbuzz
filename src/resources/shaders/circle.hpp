#pragma once

#include "../uniforms/camera.hpp"
#include "../uniforms/time.hpp"
#include "../uniforms/window.hpp"
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/resources/manager.hpp>
#include <string>

inline Physbuzz::ShaderPipelineResource shaderCircle = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .fragment = {.file = {.path = "resources/shaders/circle/circle.frag"}},
    .setup = [](Physbuzz::ShaderPipelineResource *pipeline) {
        Physbuzz::ResourceRegistry::get<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->bindPipeline(pipeline, 1);
        Physbuzz::ResourceRegistry::get<Physbuzz::UniformBufferResource<UniformTime>>("time")->bindPipeline(pipeline, 2);
        Physbuzz::ResourceRegistry::get<Physbuzz::UniformBufferResource<UniformWindow>>("window")->bindPipeline(pipeline, 3);
    },
    .draw = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);
        const Physbuzz::ModelResource *model = Physbuzz::ResourceRegistry::get<Physbuzz::ModelResource>(render.model);

        // draw meshes
        for (const Physbuzz::Mesh &mesh : model->getMeshs()) {
            mesh.bind();
            mesh.draw();
            mesh.unbind();
        }
    },
}};
