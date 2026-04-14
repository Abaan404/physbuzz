#include "shadow.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../graphics/layout.hpp"
#include "components/camera.hpp"
#include "components/lights.hpp"
#include "nodes/lights.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool PipelineShadow::Directional::build(const glm::uvec2 &resolution) {
    if (ResourceRegistry<GraphicsPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<Attachment>::contains(ResourceAttachment)) {
        success &= ResourceRegistry<Attachment>::insert(
            ResourceAttachment,
            {{
                .type = Attachment::Type::Dim2D,
                .sampler = {{
                    .type = Sampler::Type::Linear,
                }},
                .usage = Attachment::Usage::Depth,
                .format = Attachment::Format::eD32Sfloat,
            }},
            resolution);
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // lights
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

    success &= ResourceRegistry<GraphicsPipeline>::insert(
        Resource,
        {
            {
                .module = "builtin/render/shadow/directional",
            },
            {
                .description = &Model::Vertex::Description,
                .rasterization = {
                    .cullMode = vk::CullModeFlagBits::eFront,
                },
                .blend = {
                    .attachments = {{}},
                },
                .formats = {
                    .color = {},
                    .depth = GraphicsPipeline::Format::eD32Sfloat,
                },
                .layouts = {
                    .resources = {
                        ResourceLayoutFrame,
                    },
                },
                .inputs = {
                    .colors = {},
                },
            },
        });

    return success;
}

bool PipelineShadow::Point::build(const glm::uvec2 &resolution) {
    if (ResourceRegistry<GraphicsPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<Attachment>::contains(ResourceAttachment)) {
        success &= ResourceRegistry<Attachment>::insert(
            ResourceAttachment,
            {{
                .type = Attachment::Type::Cube,
                .sampler = {{
                    .type = Sampler::Type::Linear,
                }},
                .usage = Attachment::Usage::Depth,
                .format = Attachment::Format::eD32Sfloat,
            }},
            resolution);
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // lights
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

    success &= ResourceRegistry<GraphicsPipeline>::insert(
        Resource,
        {
            {
                .module = "builtin/render/shadow/point",
            },
            {
                .description = &Model::Vertex::Description,
                .rasterization = {
                    .cullMode = vk::CullModeFlagBits::eFront,
                },
                .blend = {
                    .attachments = {{}},
                },
                .formats = {
                    .color = {},
                    .depth = GraphicsPipeline::Format::eD32Sfloat,
                    .viewMask = 0x3F,
                },
                .layouts = {
                    .resources = {
                        ResourceLayoutFrame,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eAll,
                            .size = sizeof(PushConstants),
                        },
                    },
                },
                .inputs = {
                    .colors = {},
                },
            },
        });

    return success;
}

} // namespace Builtin

ShadowRenderer::ShadowRenderer(const Info &info)
    : m_Info(info) {}

bool ShadowRenderer::build() {
    bool success = true;

    success &= Builtin::PipelineShadow::Directional::build(m_Info.resolution);
    success &= Builtin::PipelineShadow::Point::build(m_Info.resolution);

    success &= m_Batch.build(m_Objects);
    // TODO combine both cullings in one pass
    success &= m_DirectionalCulling.build();
    success &= m_PointCulling.build();

    if (!success) {
        Logger::ERROR("[ShadowRenderer] Could not build shadow pipeline.");
        return false;
    }

    m_Graph.add("builtin/lights", Builtin::RenderNodeLights::build());
    m_Graph.add("builtin/shadow/batch", m_Batch.getRenderNode());
    m_Graph.merge(m_DirectionalCulling.getGraph());
    m_Graph.merge(m_PointCulling.getGraph());

    m_Graph.add(
        OutputDirectional,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            Builtin::RenderNodeLights::ResourceBufferDirectional,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            m_Batch.getIndirectBuffer(),
                            {
                                .stage = RenderNode::Stage::Indirect,
                            },
                        },
                        {
                            m_Batch.getInstanceBuffer(),
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_DirectionalCulling.getCullingBuffer(),
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                    },
                },
                .attachments = {
                    .output = {
                        {
                            Builtin::PipelineShadow::Directional::ResourceAttachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("ShadowRenderer/Directional/Execute");
                TracyVkZone(context.tracy, context.command, "ShadowRenderer/Directional");

                // FIXME this is only applied in the next tick
                {
                    const auto [_, directional] = m_Scene->getComponents<DirectionalLightComponent>().front();

                    CameraFrustum frustum = {{}};
                    frustum.update(directional.getProjectionView());

                    m_DirectionalCulling.setCamera({}, {frustum});
                }

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = Builtin::PipelineShadow::Directional::ResourceAttachment->getRingData()[context.frameInFlight].view,
                    .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

                context.command.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_Info.resolution.x), static_cast<float>(m_Info.resolution.y), 0.0f, 1.0f});
                context.command.setScissor(0, vk::Rect2D{{0, 0}, {m_Info.resolution.x, m_Info.resolution.y}});

                // issue draw calls
                context.command.beginRendering({
                    .renderArea = {
                        .offset = {0, 0},
                        .extent = {m_Info.resolution.x, m_Info.resolution.y},
                    },
                    .layerCount = 1,
                    .pDepthAttachment = &depthAttachment,
                    .pStencilAttachment = {},
                });

                // bind resources
                Builtin::PipelineShadow::Directional::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineShadow::Directional::Resource);

                // draw
                m_Batch.draw(context);

                context.command.endRendering();
            },
        });

    m_Graph.add(
        OutputPoint,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            Builtin::RenderNodeLights::ResourceBufferPoint,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            m_Batch.getIndirectBuffer(),
                            {
                                .stage = RenderNode::Stage::Indirect,
                            },
                        },
                        {
                            m_Batch.getInstanceBuffer(),
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_PointCulling.getCullingBuffer(),
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                    },
                },
                .attachments = {
                    .output = {
                        {
                            Builtin::PipelineShadow::Point::ResourceAttachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("ShadowRenderer/Point/Execute");
                TracyVkZone(context.tracy, context.command, "ShadowRenderer/Point");

                // FIXME this is only applied in the next tick
                {
                    const auto [_, point] = m_Scene->getComponents<PointLightComponent>().front();

                    std::vector<CameraFrustum> frustums;
                    frustums.reserve(6);

                    for (const auto &projectionView : point.getProjectionView()) {
                        CameraFrustum frustum = {{}};
                        frustum.update(projectionView);
                        frustums.emplace_back(frustum);
                    }

                    m_PointCulling.setCamera({}, frustums);
                }

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = Builtin::PipelineShadow::Point::ResourceAttachment->getRingData()[context.frameInFlight].view,
                    .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

                std::size_t objectCount = m_Batch.getInstanceBuffer()->getSize(context.frameInFlight) / sizeof(BatchGenerator::InstanceData);
                Builtin::PipelineShadow::Point::PushConstants pushConstants = {
                    .objectCount = static_cast<std::uint32_t>(objectCount),
                };

                Builtin::PipelineShadow::Point::Resource->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

                context.command.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_Info.resolution.x), static_cast<float>(m_Info.resolution.y), 0.0f, 1.0f});
                context.command.setScissor(0, vk::Rect2D{{0, 0}, {m_Info.resolution.x, m_Info.resolution.y}});

                // issue draw calls
                context.command.beginRendering({
                    .renderArea = {
                        .offset = {0, 0},
                        .extent = {m_Info.resolution.x, m_Info.resolution.y},
                    },
                    .viewMask = 0x3F,
                    .pDepthAttachment = &depthAttachment,
                    .pStencilAttachment = {},
                });

                // bind resources
                Builtin::PipelineShadow::Point::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineShadow::Point::Resource);

                // draw
                m_Batch.draw(context);

                context.command.endRendering();
            },
        });

    if (success) {
        App::LayoutAllocator.write(
            Builtin::PipelineShadow::Directional::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            0);

        App::LayoutAllocator.write(
            Builtin::PipelineShadow::Directional::ResourceLayoutFrame,
            m_Batch.getInstanceBuffer(),
            1);

        App::LayoutAllocator.write(
            Builtin::PipelineShadow::Directional::ResourceLayoutFrame,
            m_DirectionalCulling.getCullingBuffer(),
            2);

        App::LayoutAllocator.write(
            Builtin::PipelineShadow::Point::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            0);

        App::LayoutAllocator.write(
            Builtin::PipelineShadow::Point::ResourceLayoutFrame,
            m_Batch.getInstanceBuffer(),
            1);

        App::LayoutAllocator.write(
            Builtin::PipelineShadow::Point::ResourceLayoutFrame,
            m_PointCulling.getCullingBuffer(),
            2);
    }

    return success;
}

bool ShadowRenderer::destroy() {
    bool success = true;

    success &= m_Batch.destroy();
    success &= m_DirectionalCulling.destroy();
    success &= m_PointCulling.destroy();

    return success;
}

const RenderGraph &ShadowRenderer::getGraph() const {
    return m_Graph;
}

const ShadowRenderer::Info &ShadowRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
