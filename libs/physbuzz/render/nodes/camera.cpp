#include "camera.hpp"

#include "../../ecs/scene.hpp"
#include "../camera.hpp"

namespace Physbuzz {

namespace Builtin {

RenderNode RenderNodeCamera::build(const ObjectID &object) {
    return {
        .description = {
            .buffers = {
                .output = {
                    {
                        ResourceBuffer,
                        {
                            {
                                .type = DynamicBuffer::Type::Constant,
                            },
                            sizeof(CameraBuffer),
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
    };
}

} // namespace Builtin

} // namespace Physbuzz
