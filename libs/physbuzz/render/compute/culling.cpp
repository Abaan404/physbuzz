#include "culling.hpp"

#include "../../app/application.hpp"
#include "../../ecs/scene.hpp"
#include "../../graphics/descriptors/dynamic.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/mesh.hpp"
#include "../../graphics/pipeline.hpp"
#include "../batch.hpp"
#include "../components/camera.hpp"
#include <format>
#include <tracy/Tracy.hpp>

namespace Physbuzz {

FrustumCulling::FrustumCulling(const Info &info)
    : m_Info(info),
      m_CameraBuffer(std::format("{}camera", info.resourceIdPrefix)),
      m_CullingBuffer(std::format("{}buffer", info.resourceIdPrefix)),
      m_Pipeline(std::format("{}pipeline", info.resourceIdPrefix)),
      m_Layout(std::format("{}layout", info.resourceIdPrefix)) {}

bool FrustumCulling::build() {
    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(m_CameraBuffer)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_CameraBuffer,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(CameraBufferData));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(m_CullingBuffer)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_CullingBuffer,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(CullingBufferData));
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(m_Layout)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            m_Layout,
            {{
                .bindings = {
                    {
                        // camera
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // indirect
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // instance
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // culling
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

        success &= App::LayoutAllocator.write(m_Layout, m_Info.indirect, 0);
        success &= App::LayoutAllocator.write(m_Layout, m_CameraBuffer, 1);
        success &= App::LayoutAllocator.write(m_Layout, m_Info.instance, 2);
        success &= App::LayoutAllocator.write(m_Layout, m_CullingBuffer, 3);
    }

    m_Graph.add(
        std::format("{}camera", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .output = {
                        {
                            m_CameraBuffer,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .prepare = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Node/Prepare");

                std::size_t requiredCameraSize = (m_Cameras.size() + m_Frustums.size()) * sizeof(CameraBufferData);
                if (m_CameraBuffer->getSize(context.frameInFlight) < requiredCameraSize) {
                    m_CameraBuffer->rebuild(context, requiredCameraSize);
                }
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                std::vector<CameraFrustum> frustums = m_Frustums;
                frustums.reserve(frustums.size() + m_Cameras.size());

                // fetch ecs stored cameras, these can update every frame
                for (const auto &cameraId : m_Cameras) {
                    const auto [camera] = scene->getComponent<CameraComponent>(cameraId);
                    const CameraFrustum &frustum = camera.getFrustum();

                    frustums.emplace_back(camera.getFrustum());
                }

                std::vector<CameraBufferData> planes;
                planes.reserve(m_Cameras.size() + m_Frustums.size());

                for (const auto &frustum : frustums) {
                    const CameraFrustum::Info &info = frustum.getInfo();

                    planes.emplace_back<CameraBufferData>({
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

                m_CameraBuffer->update<CameraBufferData>(context, planes);
            },
        });

    m_Graph.add(
        std::format("{}node", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_Info.indirect,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_CameraBuffer,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                        {
                            m_Info.instance,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                    .output = {
                        {
                            m_Info.indirect,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_CullingBuffer,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                },
            },
            .prepare = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Node/Prepare");

                // the culling buffer will be replicated for each camera
                std::size_t cameraCount = (m_Cameras.size() + m_Frustums.size());
                std::size_t objectCount = m_Info.instance->getSize(context.frameInFlight) / sizeof(BatchGenerator::InstanceData);
                std::size_t requiredCullingSize = objectCount * cameraCount * sizeof(CullingBufferData);

                if (m_CullingBuffer->getSize(context.frameInFlight) < requiredCullingSize) {
                    m_CullingBuffer->rebuild(context, requiredCullingSize);
                }
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Cull/Execute");
                TracyVkZone(context.tracy, context.command, "FrustumCulling");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                std::size_t objectCount = m_Info.instance->getSize(context.frameInFlight) / sizeof(BatchGenerator::InstanceData);

                PushConstants pushConstants = {
                    .objectCount = static_cast<std::uint32_t>(objectCount),
                };

                m_Pipeline->updatePushConstants(context, ComputePipeline::PushConstantsStageFlags::eCompute, std::as_bytes(std::span(&pushConstants, 1)), 0);

                m_Pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Pipeline);

                std::size_t cameraCount = (m_Cameras.size() + m_Frustums.size());
                std::size_t indirectCount = m_Info.indirect->getSize(context.frameInFlight) / sizeof(vk::DrawIndexedIndirectCommand);
                context.command.dispatch(indirectCount, cameraCount, 1);
            },
        });

    return success;
}

bool FrustumCulling::destroy() {
    bool success = true;

    m_Cameras.clear();
    m_Frustums.clear();

    success &= ResourceRegistry<ComputePipeline>::erase(m_Pipeline);
    success &= ResourceRegistry<DescriptorLayout>::erase(m_Layout);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_CameraBuffer);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_CullingBuffer);

    return success;
}

void FrustumCulling::setCamera(const std::vector<ObjectID> &cameras, const std::vector<CameraFrustum> &frustums) {
    m_Cameras = cameras;
    m_Frustums = frustums;
}

const Resource<DynamicBuffer> &FrustumCulling::getCullingBuffer() const {
    return m_CullingBuffer;
}

const RenderGraph &FrustumCulling::getGraph() const {
    return m_Graph;
}

const FrustumCulling::Info &FrustumCulling::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
