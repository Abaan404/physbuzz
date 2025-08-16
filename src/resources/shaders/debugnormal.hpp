#pragma once

#include <physbuzz/render/renderer.hpp>
#include <physbuzz/ecs/scene.hpp>

inline Physbuzz::ShaderPipeline shaderDebugNormal = {{
    // .vertex = {.file = {.path = "resources/shaders/debug/normal.vert"}},
    // .tessControl = {},
    // .tessEvaluation = {},
    // .geometry = {.file = {.path = "resources/shaders/debug/normal.geom"}},
    // .fragment = {.file = {.path = "resources/shaders/debug/normal.frag"}},
    // .compute = {},
    // .draw = [](const Physbuzz::ShaderPipeline *pipeline, const std::shared_ptr<Physbuzz::RenderCommand> &, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
    //     const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);
    //
    //     pipeline->setUniform("u_Model", render.transform.matrix);
    //
    //     // draw meshes
    //     for (const auto &[mesh, _] : render.model->getMeshs()) {
    //         mesh.draw();
    //     }
    // },
}};
