#pragma once

#include "../../objects/skybox.hpp"
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/resources/manager.hpp>

inline Physbuzz::ShaderPipelineResource shaderSkybox = {{
    .vertex = {.file = {.path = "resources/shaders/skybox/skybox.vert"}},
    .fragment = {.file = {.path = "resources/shaders/skybox/skybox.frag"}},
    .draw = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const SkyboxComponent &skybox = scene.getComponent<SkyboxComponent>(object);
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);
        skybox.cubemap->bind(true);

        for (const Physbuzz::Mesh &mesh : render.model->getMeshs()) {
            mesh.bind();
            mesh.draw();
            mesh.unbind();
        }

        skybox.cubemap->unbind(true);
    },
}};
