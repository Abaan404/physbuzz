#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/gl/units.hpp>
#include <physbuzz/render/renderer.hpp>

inline Physbuzz::ShaderPipeline shaderCircle = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/circle/circle.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipeline *pipeline, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);
        Physbuzz::Resource<Physbuzz::Texture2D> texture = {"default/diffuse"};

        pipeline->setUniform("u_Model", render.transform.matrix);
        pipeline->setUniform("u_Texture", texture->activate());

        // draw meshes
        for (const auto &[mesh, _] : render.model->getMeshs()) {
            mesh.draw();
        }
    },
}};
