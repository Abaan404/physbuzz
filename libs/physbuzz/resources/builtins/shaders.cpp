#include "shaders.hpp"

#include "../../ecs/scene.hpp"
#include "../../render/renderer.hpp"
#include "meshes.hpp"

namespace Physbuzz {

namespace Builtin {

bool Passthrough::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<ShaderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/passthrough/passthrough.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {},
            .fragment = {.file = {.path = "resources/shaders/builtin/passthrough/passthrough.frag"}},
            .compute = {},
            .draw = [](const ShaderPipeline *, Scene &, ObjectID id) {
                for (const auto &[mesh, _] : Builtin::ScreenQuad::Resource->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}

bool Depth2D::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<ShaderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/depth/2D.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {},
            .fragment = {.file = {.path = "resources/shaders/builtin/depth/2D.frag"}},
            .compute = {},
            .draw = [](const ShaderPipeline *resource, Scene &scene, ObjectID object) {
                const auto [render] = scene.getComponent<RenderComponent>(object);

                resource->setUniform("PBZ_Model", render.transform.matrix);

                for (const auto &[mesh, _] : render.model->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}

bool DepthCubemap::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<ShaderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .vertex = {.file = {.path = "resources/shaders/builtin/depth/cubemap.vert"}},
            .tessControl = {},
            .tessEvaluation = {},
            .geometry = {.file = {.path = "resources/shaders/builtin/depth/cubemap.geom"}},
            .fragment = {.file = {.path = "resources/shaders/builtin/depth/cubemap.frag"}},
            .compute = {},
            .draw = [](const ShaderPipeline *resource, Scene &scene, ObjectID object) {
                const auto [render] = scene.getComponent<RenderComponent>(object);

                resource->setUniform("PBZ_Model", render.transform.matrix);

                for (const auto &[mesh, _] : render.model->getMeshs()) {
                    mesh.draw();
                }
            },
        }});
}

} // namespace Builtin

} // namespace Physbuzz
