#include "culling.hpp"

#include "../../app/application.hpp"
#include "../../ecs/scene.hpp"
#include "../../graphics/descriptors/dynamic.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/mesh.hpp"
#include "../../graphics/pipeline.hpp"
#include "../components/camera.hpp"
#include <format>

namespace Physbuzz {

FrustumCulling::FrustumCulling(const Info &info)
    : m_Info(info),
      m_RenderGraph({}),
      m_Instance(std::format("{}instance", info.resourceIdPrefix)),
      m_State(std::format("{}state", info.resourceIdPrefix)),
      m_Pipeline(std::format("{}pipeline", info.resourceIdPrefix)),
      m_Layout(std::format("{}layout", info.resourceIdPrefix)) {}

bool FrustumCulling::build() {
    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Instance)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_Instance,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(ObjectData));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(m_State)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_State,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(StateData));
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(m_Layout)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            m_Layout,
            {{
                .bindings = {
                    {
                        // submesh
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // instance
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // indirect
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // state
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Pipeline)) {
        success &= ResourceRegistry<ComputePipeline>::insert(
            m_Pipeline,
            {
                {
                    .module = "builtin/culling/frustum",
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

        success &= App::LayoutAllocator.write(m_Layout, m_Info.sceneBuffer, 0);
        success &= App::LayoutAllocator.write(m_Layout, m_Instance, 1);
        success &= App::LayoutAllocator.write(m_Layout, m_Info.indirectBuffer, 2);
        success &= App::LayoutAllocator.write(m_Layout, m_State, 3);
    }

    m_RenderGraph.add(
        std::format("{}setup", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .output = {
                        {
                            m_State,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("RenderNodeModels/Execute");
                TracyVkZone(context.tracy, context.command, "Model");

                // reset the atomic counter
                StateData state = {
                    .instanceOffset = 0,
                };

                m_State->update(context, std::as_bytes(std::span(&state, 1)), 0);
            },
        });

    m_RenderGraph.add(
        std::format("{}output", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_Info.sceneBuffer,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_Info.indirectBuffer,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_State,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                    .output = {
                        {
                            m_Info.indirectBuffer,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_Instance,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                },
            },
            .prepare = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Node/Prepare");

                // allocating an upper bound
                std::size_t requiredInstanceSize = m_Info.sceneBuffer->getSize(context.frameInFlight);
                if (m_Instance->getSize(context.frameInFlight) < requiredInstanceSize) {
                    m_Instance->rebuild(context, requiredInstanceSize);
                }
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("FrustumCulling/Cull/Execute");
                TracyVkZone(context.tracy, context.command, "FrustumCulling");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                const auto [camera] = scene->getComponent<CameraComponent>(m_Info.camera);
                const CameraComponent::Frustum &frustum = camera.getFrustum();

                PushConstants pushConstants = {
                    .planes = {
                        glm::vec4(frustum.left.getInfo().normal, frustum.left.getInfo().distance),
                        glm::vec4(frustum.right.getInfo().normal, frustum.right.getInfo().distance),
                        glm::vec4(frustum.bottom.getInfo().normal, frustum.bottom.getInfo().distance),
                        glm::vec4(frustum.top.getInfo().normal, frustum.top.getInfo().distance),
                        glm::vec4(frustum.near.getInfo().normal, frustum.near.getInfo().distance),
                        glm::vec4(frustum.far.getInfo().normal, frustum.far.getInfo().distance),
                    },
                };

                m_Pipeline->updatePushConstants(context, ComputePipeline::PushConstantsStageFlags::eCompute, std::as_bytes(std::span(&pushConstants, 1)), 0);
                m_Pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Pipeline);

                std::size_t indirectSize = m_Info.indirectBuffer->getSize(context.frameInFlight) / sizeof(vk::DrawIndexedIndirectCommand);

                context.command.dispatch(indirectSize, 1, 1);
            },
        });

    return success;
}

bool FrustumCulling::destroy() {
    bool success = true;

    success &= ResourceRegistry<ComputePipeline>::erase(m_Pipeline);
    success &= ResourceRegistry<DescriptorLayout>::erase(m_Layout);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_Instance);

    return success;
}

const Resource<DynamicBuffer> FrustumCulling::getInstanceBuffer() const {
    return m_Instance;
}

const RenderGraph &FrustumCulling::getGraph() const {
    return m_RenderGraph;
}

const FrustumCulling::Info &FrustumCulling::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
