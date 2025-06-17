#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/gl/units.hpp>
#include <physbuzz/render/renderer.hpp>

inline Physbuzz::ShaderPipelineResource shaderCircle = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/circle/circle.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipelineResource *pipeline, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);
        Physbuzz::ResourceHandle<Physbuzz::Texture2DResource> texture = {"default/diffuse"};

        pipeline->setUniform("u_Model", render.transform.matrix);

        texture->bind();
        pipeline->setUniform("u_Texture", Physbuzz::GL::TextureUnits::activate());

        // draw meshes
        for (const auto &[mesh, _] : render.model->getMeshs()) {
            mesh.draw();
        }

        texture->unbind();

        Physbuzz::GL::TextureUnits::reset();
    },
}};
