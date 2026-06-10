#include "culling.hpp"

#include "../../app/application.hpp"
#include "../../ecs/scene.hpp"
#include "../../graphics/descriptors/dynamic.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/mesh.hpp"
#include "../../graphics/pipeline.hpp"
#include "../batch.hpp"
#include "../components/camera.hpp"
#include "../components/lights.hpp"
#include <format>
#include <tracy/Tracy.hpp>

namespace Physbuzz {

FrustumCulling::FrustumCulling(const Info &info)
    : m_Info(info),
      m_Frustum(std::format("{}camera", info.resourceIdPrefix)),
      m_VisibleInstance(std::format("{}visible", info.resourceIdPrefix)),
      m_Indirect(std::format("{}indirect", info.resourceIdPrefix)),
      m_Pipeline(std::format("{}pipeline", info.resourceIdPrefix)),
      m_Layout(std::format("{}layout", info.resourceIdPrefix)) {}

bool FrustumCulling::build() {
    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Frustum)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_Frustum,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(FrustumBufferData));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(m_VisibleInstance)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_VisibleInstance,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(CullingBufferData));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Indirect)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_Indirect,
            {{
                .type = DynamicBuffer::Type::Indirect,
            }},
            sizeof(vk::DrawIndexedIndirectCommand));
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(m_Layout)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            m_Layout,
            {{
                .bindings = {
                    {
                        // draws
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // instance
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // camera
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // visible
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // output
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    if (!ResourceRegistry<ComputePipeline>::contains(m_Pipeline)) {
        success &= ResourceRegistry<ComputePipeline>::insert(
            m_Pipeline,
            {
                {
                    .module = "builtin/compute/frustum_culling",
                },
                {
                    .layouts = {
                        .resources = {
                            m_Layout,
                        },
                        .pushConstantRanges = {
                            {
                                .stageFlags = ComputePipeline::PushConstantsStageFlags::eCompute,
                                .size = sizeof(PushConstants),
                            },
                        },
                    },
                },
            });

        success &= App::LayoutAllocator.write(m_Layout, m_Info.batch.getIndirectBuffer(), 0);
        success &= App::LayoutAllocator.write(m_Layout, m_Info.batch.getInstanceBuffer(), 1);
        success &= App::LayoutAllocator.write(m_Layout, m_Frustum, 2);
        success &= App::LayoutAllocator.write(m_Layout, m_VisibleInstance, 3);
        success &= App::LayoutAllocator.write(m_Layout, m_Indirect, 4);
    }

    m_Graph.add(
        std::format("{}camera", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .output = {
                        {
                            m_Frustum,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .prepare = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Node/Prepare");

                std::size_t frustumCount = m_Info.objects.cameras.size() +
                                           m_Info.objects.directionalLights.size() +
                                           m_Info.objects.pointLights.size() * 6 + // each point light has 6 cameras/faces for a omni/cubic view
                                           m_Info.objects.spotLights.size();

                m_FrustumBufferIds.clear();
                m_FrustumBuffer.clear();
                m_FrustumBuffer.reserve(frustumCount);

                std::uint32_t frustumIdx = 0;

                for (const auto &object : m_Info.objects.cameras) {
                    const auto [camera] = scene->getComponent<CameraComponent>(object);
                    const Frustum::Info &info = camera.getFrustum().getInfo();

                    m_FrustumBuffer.emplace_back<FrustumBufferData>({
                        .planes = {
                            glm::vec4(info.left.getInfo().normal, info.left.getInfo().distance),
                            glm::vec4(info.right.getInfo().normal, info.right.getInfo().distance),
                            glm::vec4(info.bottom.getInfo().normal, info.bottom.getInfo().distance),
                            glm::vec4(info.top.getInfo().normal, info.top.getInfo().distance),
                            glm::vec4(info.near.getInfo().normal, info.near.getInfo().distance),
                            glm::vec4(info.far.getInfo().normal, info.far.getInfo().distance),
                        },
                    });

                    m_FrustumBufferIds[object] = frustumIdx;
                    frustumIdx++;
                }

                for (const auto &object : m_Info.objects.directionalLights) {
                    const auto [directional] = scene->getComponent<DirectionalLightComponent>(object);
                    const Frustum::Info &info = directional.getFrustum().getInfo();

                    m_FrustumBuffer.emplace_back<FrustumBufferData>({
                        .planes = {
                            glm::vec4(info.left.getInfo().normal, info.left.getInfo().distance),
                            glm::vec4(info.right.getInfo().normal, info.right.getInfo().distance),
                            glm::vec4(info.bottom.getInfo().normal, info.bottom.getInfo().distance),
                            glm::vec4(info.top.getInfo().normal, info.top.getInfo().distance),
                            glm::vec4(info.near.getInfo().normal, info.near.getInfo().distance),
                            glm::vec4(info.far.getInfo().normal, info.far.getInfo().distance),
                        },
                    });

                    m_FrustumBufferIds[object] = frustumIdx;
                    frustumIdx++;
                }

                for (const auto &object : m_Info.objects.pointLights) {
                    const auto [point] = scene->getComponent<PointLightComponent>(object);
                    for (const auto &frustum : point.getFrustums()) {
                        const Frustum::Info &info = frustum.getInfo();

                        m_FrustumBuffer.emplace_back<FrustumBufferData>({
                            .planes = {
                                glm::vec4(info.left.getInfo().normal, info.left.getInfo().distance),
                                glm::vec4(info.right.getInfo().normal, info.right.getInfo().distance),
                                glm::vec4(info.bottom.getInfo().normal, info.bottom.getInfo().distance),
                                glm::vec4(info.top.getInfo().normal, info.top.getInfo().distance),
                                glm::vec4(info.near.getInfo().normal, info.near.getInfo().distance),
                                glm::vec4(info.far.getInfo().normal, info.far.getInfo().distance),
                            },
                        });
                    }

                    m_FrustumBufferIds[object] = frustumIdx;
                    frustumIdx += point.getFrustums().size();
                }

                for (const auto &object : m_Info.objects.spotLights) {
                    const auto [spot] = scene->getComponent<SpotLightComponent>(object);
                    const Frustum::Info &info = spot.getFrustum().getInfo();

                    m_FrustumBuffer.emplace_back<FrustumBufferData>({
                        .planes = {
                            glm::vec4(info.left.getInfo().normal, info.left.getInfo().distance),
                            glm::vec4(info.right.getInfo().normal, info.right.getInfo().distance),
                            glm::vec4(info.bottom.getInfo().normal, info.bottom.getInfo().distance),
                            glm::vec4(info.top.getInfo().normal, info.top.getInfo().distance),
                            glm::vec4(info.near.getInfo().normal, info.near.getInfo().distance),
                            glm::vec4(info.far.getInfo().normal, info.far.getInfo().distance),
                        },
                    });

                    m_FrustumBufferIds[object] = frustumIdx;
                    frustumIdx++;
                }

                std::size_t requiredFrustumSize = frustumCount * sizeof(FrustumBufferData);
                if (m_Frustum->getSize(context.frameInFlight) < requiredFrustumSize) {
                    m_Frustum->rebuild(context, requiredFrustumSize);
                }
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                m_Frustum->update<FrustumBufferData>(context, m_FrustumBuffer);
            },
        });

    m_Graph.add(
        std::format("{}node", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_Info.batch.getIndirectBuffer(),
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_Info.batch.getInstanceBuffer(),
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_Frustum,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                    .output = {
                        {
                            m_Indirect,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_VisibleInstance,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                },
            },
            .prepare = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Node/Prepare");

                std::size_t frustumCount = m_FrustumBuffer.size();
                std::size_t objectCount = m_Info.batch.getObjectCount(context.frameInFlight);
                std::size_t drawCount = m_Info.batch.getDrawCount(context.frameInFlight);

                // the culling buffers will be replicated for each camera
                std::size_t requiredVisibleInstanceSize = objectCount * frustumCount * sizeof(CullingBufferData);
                if (m_VisibleInstance->getSize(context.frameInFlight) < requiredVisibleInstanceSize) {
                    m_VisibleInstance->rebuild(context, requiredVisibleInstanceSize);
                }

                std::size_t requiredIndirectSize = drawCount * frustumCount * sizeof(vk::DrawIndexedIndirectCommand);
                if (m_Indirect->getSize(context.frameInFlight) < requiredIndirectSize) {
                    m_Indirect->rebuild(context, requiredIndirectSize);
                }
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Cull/Execute");
                TracyVkZone(context.tracy, context.command, "FrustumCulling");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                std::size_t frustumCount = m_FrustumBuffer.size();
                std::size_t objectCount = m_Info.batch.getObjectCount(context.frameInFlight);
                std::size_t drawCount = m_Info.batch.getDrawCount(context.frameInFlight);

                PushConstants pushConstants = {
                    .objectCount = static_cast<std::uint32_t>(objectCount),
                    .drawCount = static_cast<std::uint32_t>(drawCount),
                };

                m_Pipeline->updatePushConstants(context, ComputePipeline::PushConstantsStageFlags::eCompute, std::as_bytes(std::span(&pushConstants, 1)), 0);
                m_Pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Pipeline);

                context.command.dispatch(drawCount, frustumCount, 1);
            },
        });

    return success;
}

bool FrustumCulling::destroy() {
    bool success = true;

    m_FrustumBufferIds.clear();
    m_FrustumBuffer.clear();

    success &= ResourceRegistry<ComputePipeline>::erase(m_Pipeline);
    success &= ResourceRegistry<DescriptorLayout>::erase(m_Layout);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_Frustum);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_VisibleInstance);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_Indirect);

    return success;
}

void FrustumCulling::setObjects(const Objects &objects) {
    m_Info.objects = objects;
}

const Resource<DynamicBuffer> &FrustumCulling::getVisibleInstanceBuffer() const {
    return m_VisibleInstance;
}

const Resource<DynamicBuffer> &FrustumCulling::getIndirectBuffer() const {
    return m_Indirect;
}

std::uint32_t FrustumCulling::getFrustumId(ObjectID object) const {
    return m_FrustumBufferIds.at(object);
}

std::uint64_t FrustumCulling::getIndirectOffset(std::uint32_t frustumId, std::uint32_t frameInFlight) const {
    return frustumId * m_Info.batch.getDrawCount(frameInFlight) * sizeof(vk::DrawIndexedIndirectCommand);
}

const RenderGraph &FrustumCulling::getGraph() const {
    return m_Graph;
}

const FrustumCulling::Info &FrustumCulling::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
