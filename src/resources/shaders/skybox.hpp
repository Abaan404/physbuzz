#pragma once

#include "../../objects/skybox.hpp"
#include <physbuzz/render/gl/depth.hpp>
#include <physbuzz/render/gl/units.hpp>
#include <physbuzz/render/renderer.hpp>

inline Physbuzz::ShaderPipelineResource shaderSkybox = {{
    .vertex = {.file = {.path = "resources/shaders/skybox/skybox.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/skybox/skybox.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipelineResource *pipeline, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const auto [render, skybox] = scene.getComponent<Physbuzz::RenderComponent, SkyboxComponent>(object);

        Physbuzz::GL::DepthFunc depthFunc = Physbuzz::GL::getDepthFunc();

        Physbuzz::GL::setDepthMask(false);
        Physbuzz::GL::setDepthFunc(Physbuzz::GL::DepthFunc::LEqual);

        pipeline->setUniform("u_Skybox", Physbuzz::GL::TextureUnits::activate());

        skybox.cubemap->bind();

        for (const auto &[mesh, _] : render.model->getMeshs()) {
            mesh.draw();
        }

        skybox.cubemap->unbind();

        Physbuzz::GL::setDepthMask(true);
        Physbuzz::GL::setDepthFunc(depthFunc);
    },
}};
