#include "lights.hpp"

#include "../../ecs/scene.hpp"
#include "../lighting.hpp"

namespace Physbuzz {

namespace Builtin {

RenderGraph RenderNodeLights::build() {
    RenderGraph graph = {{}};

    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferDirectional)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferDirectional,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(DirectionalLightComponent));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferPoint)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferPoint,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(PointLightComponent));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferSpot)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferSpot,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(SpotLightComponent));
    }

    if (!success) {
        Logger::ERROR("[RenderNodeLights] Failed to build buffers");
        return graph;
    }

    std::shared_ptr<std::vector<DirectionalLightComponent>> directionals = std::make_shared<std::vector<DirectionalLightComponent>>();
    std::shared_ptr<std::vector<PointLightComponent>> points = std::make_shared<std::vector<PointLightComponent>>();
    std::shared_ptr<std::vector<SpotLightComponent>> spots = std::make_shared<std::vector<SpotLightComponent>>();

    graph.add(
        Id,
        {
            .description = {
                .buffers = {
                    .output = {
                        {
                            ResourceBufferDirectional,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                        {
                            ResourceBufferPoint,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                        {
                            ResourceBufferSpot,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .prepare = [](Scene *scene, const RenderContext &context) {
                std::size_t requiredSizeDirectional = scene->getComponentArray<DirectionalLightComponent>().size() * sizeof(DirectionalLightComponent);
                std::size_t requiredSizePoint = scene->getComponentArray<PointLightComponent>().size() * sizeof(PointLightComponent);
                std::size_t requiredSizeSpot = scene->getComponentArray<SpotLightComponent>().size() * sizeof(SpotLightComponent);

                if (ResourceBufferDirectional->getSize(context.frameInFlight) < requiredSizeDirectional) {
                    ResourceBufferDirectional->rebuild(context, requiredSizeDirectional);
                }

                if (ResourceBufferPoint->getSize(context.frameInFlight) < requiredSizePoint) {
                    ResourceBufferPoint->rebuild(context, requiredSizePoint);
                }

                if (ResourceBufferSpot->getSize(context.frameInFlight) < requiredSizeSpot) {
                    ResourceBufferSpot->rebuild(context, requiredSizeSpot);
                }
            },
            .execute = [](Scene *scene, const RenderContext &context) {
                ResourceBufferDirectional->update(context, scene->getComponentArray<DirectionalLightComponent>());
                ResourceBufferPoint->update(context, scene->getComponentArray<PointLightComponent>());
                ResourceBufferSpot->update(context, scene->getComponentArray<SpotLightComponent>());
            },
        });

    return graph;
}

} // namespace Builtin

} // namespace Physbuzz
