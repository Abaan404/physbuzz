#pragma once

#include <physbuzz/render/renderer.hpp>

inline Physbuzz::ShaderPipelineResource shaderCube = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/cube/cube.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipelineResource *pipeline, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);

        pipeline->setUniform("u_Model", render.transform.matrix);

        // draw meshes
        for (const auto &[mesh, _] : render.model->getMeshs()) {
            mesh.draw();
        }
    },
}};
