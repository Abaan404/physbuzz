#pragma once

#include "../../objects/skybox.hpp"
#include <physbuzz/render/renderer.hpp>

inline Physbuzz::RenderPipeline shaderSkybox = {{
    // .vertex = {.file = {.path = "resources/shaders/skybox/skybox.vert"}},
    // .tessControl = {},
    // .tessEvaluation = {},
    // .geometry = {},
    // .fragment = {.file = {.path = "resources/shaders/skybox/skybox.frag"}},
    // .compute = {},
    // .draw = [](const Physbuzz::ShaderPipeline *pipeline, const std::shared_ptr<Physbuzz::RenderCommand> &, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
    //     const auto [render, skybox] = scene.getComponent<Physbuzz::RenderComponent, SkyboxComponent>(object);
    //
    //     pipeline->setUniform("u_Skybox", skybox.cubemap->activate());
    //
    //     for (const auto &[mesh, _] : render.model->getMeshs()) {
    //         mesh.draw();
    //     }
    // },
}};
