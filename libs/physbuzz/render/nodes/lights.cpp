#include "lights.hpp"

#include "../../ecs/scene.hpp"
#include "../lighting.hpp"

namespace Physbuzz {

namespace Builtin {

RenderNode RenderNodeLights::build() {
    return {
        .description = {
            .buffers = {
                .output = {
                    {
                        ResourceBufferDirectional,
                        {
                            {
                                .type = DynamicBuffer::Type::Structured,
                            },
                            sizeof(DirectionalLightComponent),
                        },
                    },
                    {
                        ResourceBufferPoint,
                        {
                            {
                                .type = DynamicBuffer::Type::Structured,
                            },
                            sizeof(PointLightComponent),
                        },
                    },
                    {
                        ResourceBufferSpot,
                        {
                            {
                                .type = DynamicBuffer::Type::Structured,
                            },
                            sizeof(SpotLightComponent),
                        },
                    },
                },
            },
        },
        .execute = [&](Scene *scene, const RenderContext &context) {
            ResourceBufferDirectional->update(context, scene->getComponentArray<DirectionalLightComponent>());
            ResourceBufferPoint->update(context, scene->getComponentArray<PointLightComponent>());
            ResourceBufferSpot->update(context, scene->getComponentArray<SpotLightComponent>());
        },
    };
}

} // namespace Builtin

} // namespace Physbuzz
