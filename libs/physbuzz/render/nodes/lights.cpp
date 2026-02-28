#include "lights.hpp"

#include "../../ecs/scene.hpp"
#include "../components/lights.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Physbuzz {

namespace Builtin {

RenderNode RenderNodeLights::build() {
    RenderNode node = {{}};

    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferDirectional)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferDirectional,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(DirectionalLightBuffer));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferPoint)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferPoint,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(PointLightBuffer));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferSpot)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferSpot,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(SpotLightBuffer));
    }

    if (!success) {
        Logger::ERROR("[RenderNodeLights] Failed to build buffers");
        return node;
    }

    std::shared_ptr<std::vector<DirectionalLightBuffer>> directionals = std::make_shared<std::vector<DirectionalLightBuffer>>();
    std::shared_ptr<std::vector<PointLightBuffer>> points = std::make_shared<std::vector<PointLightBuffer>>();
    std::shared_ptr<std::vector<SpotLightBuffer>> spots = std::make_shared<std::vector<SpotLightBuffer>>();

    node = {
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
        .prepare = [directionals, points, spots](Scene *scene, const RenderContext &context) {
            std::vector<DirectionalLightComponent> directionalComponents = scene->getComponentArray<DirectionalLightComponent>();
            std::size_t requiredSizeDirectionals = directionalComponents.size() * sizeof(DirectionalLightBuffer);

            if (ResourceBufferDirectional->getSize(context.frameInFlight) < requiredSizeDirectionals) {
                ResourceBufferDirectional->rebuild(context, requiredSizeDirectionals);
            }

            directionals->clear();
            directionals->reserve(directionalComponents.size());
            for (const auto &directional : directionalComponents) {
                const DirectionalLightComponent::Info &info = directional.getInfo();

                directionals->emplace_back<DirectionalLightBuffer>({
                    .projectionView = directional.getProjectionView(),
                    .direction = info.direction,
                    .intensity = info.intensity,
                });
            }

            std::vector<PointLightComponent> pointComponents = scene->getComponentArray<PointLightComponent>();
            std::size_t requiredSizePoints = pointComponents.size() * sizeof(PointLightBuffer);

            if (ResourceBufferPoint->getSize(context.frameInFlight) < requiredSizePoints) {
                ResourceBufferPoint->rebuild(context, requiredSizePoints);
            }

            points->clear();
            points->reserve(pointComponents.size());
            for (const auto &point : pointComponents) {
                const PointLightComponent::Info &info = point.getInfo();

                points->emplace_back<PointLightBuffer>({
                    .projectionView = point.getProjectionView(),
                    .position = info.position,
                    .depth = point.getInfo().depth,
                    .intensity = info.intensity,
                });
            }

            std::vector<SpotLightComponent> spotComponents = scene->getComponentArray<SpotLightComponent>();
            std::size_t requiredSizeSpots = spotComponents.size() * sizeof(SpotLightBuffer);

            if (ResourceBufferSpot->getSize(context.frameInFlight) < requiredSizeSpots) {
                ResourceBufferSpot->rebuild(context, requiredSizeSpots);
            }

            spots->clear();
            spots->reserve(spotComponents.size());
            for (const auto &spot : spotComponents) {
                const SpotLightComponent::Info &info = spot.getInfo();

                spots->emplace_back<SpotLightBuffer>({
                    .projectionView = {},
                    .position = info.position,
                    .direction = info.direction,
                    .intensity = info.intensity,
                    .cutOff = info.cutOff,
                    .outerCutOff = info.outerCutOff,
                });
            }
        },
        .execute = [directionals, points, spots](Scene *scene, const RenderContext &context) {
            ResourceBufferDirectional->update(context, *directionals);
            ResourceBufferPoint->update(context, *points);
            ResourceBufferSpot->update(context, *spots);
        },
    };

    return node;
}

} // namespace Builtin

} // namespace Physbuzz
