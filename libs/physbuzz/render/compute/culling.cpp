#include "culling.hpp"

#include "../../app/application.hpp"
#include "../../ecs/scene.hpp"
#include "../../graphics/descriptors/dynamic.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/mesh.hpp"
#include "../../graphics/pipeline.hpp"
#include "../components/camera.hpp"
#include <format>
#include <tracy/Tracy.hpp>

namespace Physbuzz {

FrustumCulling::FrustumCulling(const Info &info)
    : m_Info(info),
      m_Buffer(std::format("{}buffer", info.resourceIdPrefix)),
      m_Pipeline(std::format("{}pipeline", info.resourceIdPrefix)),
      m_Layout(std::format("{}layout", info.resourceIdPrefix)) {}

bool FrustumCulling::build() {
    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Buffer)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_Buffer,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(BufferData));
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(m_Layout)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            m_Layout,
            {{
                .bindings = {
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

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Pipeline)) {
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
        success &= App::LayoutAllocator.write(m_Layout, m_Info.instance, 1);
        success &= App::LayoutAllocator.write(m_Layout, m_Buffer, 2);
    }

    m_RenderNode = {
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
                        m_Buffer,
                        {
                            .stage = RenderNode::Stage::Compute,
                        },
                    },
                },
            },
        },
        .prepare = [this](Scene *scene, const RenderContext &context) {
            ZoneScopedN("FrustumCulling/Node/Prepare");

            std::size_t requiredBufferSize = m_Info.instance->getSize(context.frameInFlight);
            if (m_Buffer->getSize(context.frameInFlight) < requiredBufferSize) {
                m_Buffer->rebuild(context, requiredBufferSize);
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

            std::size_t indirectSize = m_Info.indirect->getSize(context.frameInFlight) / sizeof(vk::DrawIndexedIndirectCommand);

            context.command.dispatch(indirectSize, 1, 1);
        },
    };

    return success;
}

bool FrustumCulling::destroy() {
    bool success = true;

    success &= ResourceRegistry<ComputePipeline>::erase(m_Pipeline);
    success &= ResourceRegistry<DescriptorLayout>::erase(m_Layout);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_Buffer);

    return success;
}

const Resource<DynamicBuffer> FrustumCulling::getBuffer() const {
    return m_Buffer;
}

const RenderNode &FrustumCulling::getRenderNode() const {
    return m_RenderNode;
}

const FrustumCulling::Info &FrustumCulling::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
