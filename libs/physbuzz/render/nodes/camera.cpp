#include "camera.hpp"

#include "../../ecs/scene.hpp"
#include "../camera.hpp"

namespace Physbuzz {

namespace Builtin {

RenderGraph RenderNodeCamera::build(const ObjectID &object) {
    RenderGraph graph = {{}};

    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBuffer)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBuffer,
            {{
                .type = DynamicBuffer::Type::Constant,
            }},
            sizeof(CameraBuffer));
    }

    if (!success) {
        Logger::ERROR("[RenderNodeCamera] Failed to build buffers");
        return graph;
    }

    graph.add(
        Id,
        {
            .description = {
                .buffers = {
                    .output = {
                        {
                            ResourceBuffer,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .execute = [&object](Scene *scene, const RenderContext &context) {
                const auto [camera] = scene->getComponent<CameraComponent>(object);

                std::vector<CameraBuffer> buffer = {{
                    .position = camera.getInfo().view.position,
                    .view = camera.getView(),
                    .projection = camera.getProjection(),
                }};

                ResourceBuffer->update(context, buffer);
            },
        });

    return graph;
}

} // namespace Builtin

} // namespace Physbuzz
