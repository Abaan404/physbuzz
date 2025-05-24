#pragma once

#include <physbuzz/render/model.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/resources/manager.hpp>

inline Physbuzz::ShaderPipelineResource shaderDebugNormal = {{
    .vertex = {.file = {.path = "resources/shaders/debug/normal.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {.file = {.path = "resources/shaders/debug/normal.geom"}},
    .fragment = {.file = {.path = "resources/shaders/debug/normal.frag"}},
    .compute = {},
    .draw = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);

        // draw meshes
        for (const Physbuzz::Mesh &mesh : render.model->getMeshs()) {
            mesh.bind();
            mesh.draw();
            mesh.unbind();
        }
    },
}};
