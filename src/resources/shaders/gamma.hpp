#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/gl/capabilities.hpp>
#include <physbuzz/render/gl/units.hpp>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/resources/builtins/meshes.hpp>

inline Physbuzz::ShaderPipeline shaderGamma = {{
    .vertex = {.file = {.path = "resources/shaders/gamma/gamma.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/gamma/gamma.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipeline *, Physbuzz::Scene &, Physbuzz::ObjectID) {
        for (const auto &[mesh, _] : Physbuzz::Builtin::MeshScreenQuad::Resource->getMeshs()) {
            mesh.draw();
        }
    },
}};
