#include "shaders.hpp"

#include "../../ecs/scene.hpp"
#include "../../render/renderer.hpp"
#include "meshes.hpp"

namespace Physbuzz {

namespace Builtin {

bool Passthrough::build() {
    if (ResourceRegistry<ShaderPipelineResource>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<ShaderPipelineResource>::insert(
        Resource.getIdentifier(),
        {{.vertex = {.file = {.path = "resources/shaders/builtin/passthrough/passthrough.vert"}},
          .tessControl = {},
          .tessEvaluation = {},
          .geometry = {},
          .fragment = {.file = {.path = "resources/shaders/builtin/passthrough/passthrough.frag"}},
          .compute = {},
          .draw = [](const ShaderPipelineResource *, Scene &, ObjectID id) {
              for (const auto &[mesh, _] : Builtin::ScreenQuad::Resource->getMeshs()) {
                  mesh.draw();
              }
          }}});
}

bool Depth::build() {
    if (ResourceRegistry<ShaderPipelineResource>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<ShaderPipelineResource>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/shadow/shadow.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {},
            .fragment = {.file = {.path = "resources/shaders/builtin/shadow/shadow.frag"}},
            .compute = {},
            .draw = [](const ShaderPipelineResource *resource, Scene &scene, ObjectID object) {
                const auto [render] = scene.getComponent<RenderComponent>(object);

                resource->setUniform("u_Model", render.transform.matrix);

                for (const auto &[mesh, _] : render.model->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}
} // namespace Builtin

} // namespace Physbuzz
