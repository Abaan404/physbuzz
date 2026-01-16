#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/graphics/renderer.hpp>

inline Physbuzz::RenderPipeline shaderCircle = {{
    // .vertex = {.file = {.path = "resources/shaders/circle/circle.vert"}},
    // .tessControl = {},
    // .tessEvaluation = {},
    // .geometry = {},
    // .fragment = {.file = {.path = "resources/shaders/circle/circle.frag"}},
    // .compute = {},
    // .draw = [](const Physbuzz::ShaderPipeline *pipeline, const std::shared_ptr<Physbuzz::RenderCommand> &, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
    //     const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);
    //     Physbuzz::Resource<Physbuzz::Texture2D> texture = {"default/diffuse"};
    //
    //     pipeline->setUniform("u_Model", render.transform.matrix);
    //     pipeline->setUniform("u_Texture", texture->activate());
    //
    //     // draw meshes
    //     for (const auto &[mesh, _] : render.model->getMeshs()) {
    //         mesh.draw();
    //     }
    // },
}};
