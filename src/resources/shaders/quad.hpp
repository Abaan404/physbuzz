#pragma once

#include <physbuzz/render/renderer.hpp>
#include <physbuzz/ecs/scene.hpp>

inline Physbuzz::ShaderPipeline shaderQuad = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/quad/quad.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipeline *pipeline, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);

        pipeline->setUniform("u_Model", render.transform.matrix);

        // draw meshes
        for (const auto &[mesh, _] : render.model->getMeshs()) {
            mesh.draw();
        }
    },
}};
