#pragma once

#include <physbuzz/render/model.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/resources/handle.hpp>

inline Physbuzz::ShaderPipelineResource shaderCircle = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .fragment = {.file = {.path = "resources/shaders/circle/circle.frag"}},
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
