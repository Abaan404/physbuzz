#include "resources.hpp"

#include "../ecs/scene.hpp"
#include "../ecs/system.hpp"
#include "../graphics/descriptors/dynamic.hpp"
#include "components/camera.hpp"
#include "components/lights.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

ResourceContext::ResourceContext(Scene *scene)
    : System(scene) {}

bool ResourceContext::build() {
    auto camera = ResourceRegistry<DynamicBuffer>::insert(
        "builtin/camera",
        {{
            .type = DynamicBuffer::Type::Constant,
        }},
        sizeof(CameraBufferData));

    if (!camera) {
        return false;
    }

    auto directionalLights = ResourceRegistry<DynamicBuffer>::insert(
        "builtin/lights/directional",
        {{
            .type = DynamicBuffer::Type::Structured,
        }},
        sizeof(DirectionalLightBufferData));

    if (!directionalLights) {
        return false;
    }

    auto pointLights = ResourceRegistry<DynamicBuffer>::insert(
        "builtin/lights/point",
        {{
            .type = DynamicBuffer::Type::Structured,
        }},
        sizeof(PointLightBufferData));

    if (!pointLights) {
        return false;
    }

    auto spotLights = ResourceRegistry<DynamicBuffer>::insert(
        "builtin/lights/spot",
        {{
            .type = DynamicBuffer::Type::Structured,
        }},
        sizeof(SpotLightBufferData));

    if (!spotLights) {
        return false;
    }

    m_Resources = {
        .camera = std::move(*camera),
        .directionalLights = std::move(*directionalLights),
        .pointLights = std::move(*pointLights),
        .spotLights = std::move(*spotLights),
    };

    return true;
}

bool ResourceContext::destroy() {
    return true;
}

RenderNode ResourceContext::buildCameraNode() {
    return {
        .description = {
            .buffers = {
                .output = {
                    {
                        m_Resources.camera,
                        {
                            .stage = RenderNode::Stage::Transfer,
                        },
                    },
                },
            },
        },
        .execute = [this](const RenderContext &context) {
            ZoneScopedN("RenderNodeCamera/Execute");
            TracyVkZone(context.tracy, context.command, "Camera");

            const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.cameraId);

            std::vector<CameraBufferData> buffer = {{
                .position = camera.getInfo().view.position,
                .view = camera.getView(),
                .projection = camera.getProjection(),
            }};

            m_Resources.camera->update(context, buffer);
        },
    };
}

RenderNode ResourceContext::buildLightsNode() {
    return {
        .description = {
            .buffers = {
                .output = {
                    {
                        m_Resources.directionalLights,
                        {
                            .stage = RenderNode::Stage::Transfer,
                        },
                    },
                    {
                        m_Resources.pointLights,
                        {
                            .stage = RenderNode::Stage::Transfer,
                        },
                    },
                    {
                        m_Resources.spotLights,
                        {
                            .stage = RenderNode::Stage::Transfer,
                        },
                    },
                },
            },
        },
        .prepare = [this](const RenderContext &context) {
            ZoneScopedN("RenderNodeLights/Prepare");

            // gather all components
            const std::vector<DirectionalLightComponent> &directionalComponents = m_Scene->getComponentArray<DirectionalLightComponent>();
            std::size_t requiredSizeDirectionals = directionalComponents.size() * sizeof(DirectionalLightBufferData);

            const std::vector<PointLightComponent> &pointComponents = m_Scene->getComponentArray<PointLightComponent>();
            std::size_t requiredSizePoints = pointComponents.size() * sizeof(PointLightBufferData);

            const std::vector<SpotLightComponent> &spotComponents = m_Scene->getComponentArray<SpotLightComponent>();
            std::size_t requiredSizeSpots = spotComponents.size() * sizeof(SpotLightBufferData);

            // reset cpu and gpu buffers
            directionalLightsBuffer.clear();
            directionalLightsBuffer.reserve(directionalComponents.size());

            if (m_Resources.directionalLights->getSize(context.frameInFlight) < requiredSizeDirectionals) {
                m_Resources.directionalLights->rebuild(context, requiredSizeDirectionals);
            }

            pointLightsBuffer.clear();
            pointLightsBuffer.reserve(pointComponents.size());

            if (m_Resources.pointLights->getSize(context.frameInFlight) < requiredSizePoints) {
                m_Resources.pointLights->rebuild(context, requiredSizePoints);
            }

            spotLightsBuffer.clear();
            spotLightsBuffer.reserve(spotComponents.size());

            if (m_Resources.spotLights->getSize(context.frameInFlight) < requiredSizeSpots) {
                m_Resources.spotLights->rebuild(context, requiredSizeSpots);
            }

            // populate buffers
            for (const auto &directional : directionalComponents) {
                const DirectionalLightComponent::Info &info = directional.getInfo();

                directionalLightsBuffer.emplace_back<DirectionalLightBufferData>({
                    .projectionView = directional.getProjectionView(),
                    .direction = info.direction,
                    .intensity = info.intensity,
                });
            }

            for (const auto &point : pointComponents) {
                const PointLightComponent::Info &info = point.getInfo();

                pointLightsBuffer.emplace_back<PointLightBufferData>({
                    .projectionView = point.getProjectionViews(),
                    .position = info.position,
                    .depth = point.getInfo().depth,
                    .intensity = info.intensity,
                });
            }

            for (const auto &spot : spotComponents) {
                const SpotLightComponent::Info &info = spot.getInfo();

                spotLightsBuffer.emplace_back<SpotLightBufferData>({
                    .projectionView = {},
                    .position = info.position,
                    .direction = info.direction,
                    .intensity = info.intensity,
                    .cutOff = info.cutOff,
                    .outerCutOff = info.outerCutOff,
                });
            }
        },
        .execute = [this](const RenderContext &context) {
            ZoneScopedN("RenderNodeLights/Execute");
            TracyVkZone(context.tracy, context.command, "Light");

            // write buffers
            m_Resources.directionalLights->update(context, directionalLightsBuffer);
            m_Resources.pointLights->update(context, pointLightsBuffer);
            m_Resources.spotLights->update(context, spotLightsBuffer);
        },
    };
}

void ResourceContext::setCamera(ObjectID camera) {
    m_Info.cameraId = camera;
}

const ResourceContext::Info &ResourceContext::getInfo() const {
    return m_Info;
}

const ResourceContext::Resources &ResourceContext::getResouces() const {
    return m_Resources;
}

} // namespace Physbuzz
