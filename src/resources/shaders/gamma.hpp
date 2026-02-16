#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/graphics/pipeline.hpp>
#include <physbuzz/graphics/renderer.hpp>
#include <physbuzz/render/lighting.hpp>

inline Physbuzz::RenderPipeline shaderGamma = {{
    // .vertex = {.file = {.path = "resources/shaders/gamma/gamma.vert"}},
    // .tessControl = {},
    // .tessEvaluation = {},
    // .geometry = {},
    // .fragment = {.file = {.path = "resources/shaders/gamma/gamma.frag"}},
    // .compute = {},
    // .draw = [](const Physbuzz::ShaderPipeline *, const std::shared_ptr<Physbuzz::RenderCommand> &, Physbuzz::Scene &, Physbuzz::ObjectID ) {
    //     for (const auto &[mesh, _] : Physbuzz::Builtin::MeshRendererScreenQuad::Resource->getMeshs()) {
    //         mesh.draw();
    //     }
    // },
}};
