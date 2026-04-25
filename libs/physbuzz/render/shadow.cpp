#include "shadow.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../graphics/layout.hpp"
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
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eVertex,
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
                },
                .layouts = {
                    .resources = {
                        ResourceLayoutFrame,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eVertex,
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
    success &= m_Culling.build();

    const auto updateCullingObjects = [this](...) {
        m_Culling.setObjects({
            .directionalLights = m_Scene->getObjects<DirectionalLightComponent>(),
            .pointLights = m_Scene->getObjects<PointLightComponent>(),
            .spotLights = m_Scene->getObjects<SpotLightComponent>(),
        });
    };

    m_Events = {
        m_Scene->addCallback<OnComponentSetEvent<DirectionalLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentSetEvent<PointLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentSetEvent<SpotLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentEraseEvent<DirectionalLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentEraseEvent<PointLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentEraseEvent<SpotLightComponent>>(updateCullingObjects),
    };

    if (!success) {
        Logger::ERROR("[ShadowRenderer] Could not build shadow pipeline.");
        return false;
    }

    m_Graph.add("builtin/lights", Builtin::RenderNodeLights::build());
    m_Graph.add("builtin/shadow/batch", m_Batch.getRenderNode());
    m_Graph.merge(m_Culling.getGraph());

    m_Graph.add(
        OutputDirectional,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_Culling.getIndirectBuffer(),
                            {
                                .stage = RenderNode::Stage::Indirect,
                            },
                        },
                        {
                            Builtin::RenderNodeLights::ResourceBufferDirectional,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            m_Batch.getInstanceBuffer(),
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_Culling.getVisibleInstanceBuffer(),
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

                // TODO multiple objects
                ObjectID object = *m_Scene->getObjects<DirectionalLightComponent>().begin();

                // bind resources
                Builtin::PipelineShadow::Directional::PushConstants pushConstants = {
                    .frustumOffset = static_cast<std::uint32_t>(m_Culling.getFrustumOffset(object)),
                };

                Builtin::PipelineShadow::Directional::Resource->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eVertex, std::as_bytes(std::span(&pushConstants, 1)), 0);
                Builtin::PipelineShadow::Directional::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineShadow::Directional::Resource);

                // draw
                m_Batch.draw(context, m_Culling.getIndirectBuffer(), m_Culling.getFrustumOffset(object) * sizeof(vk::DrawIndexedIndirectCommand));

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
                            m_Culling.getIndirectBuffer(),
                            {
                                .stage = RenderNode::Stage::Indirect,
                            },
                        },
                        {
                            Builtin::RenderNodeLights::ResourceBufferPoint,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            m_Batch.getInstanceBuffer(),
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_Culling.getVisibleInstanceBuffer(),
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

                std::size_t drawCount = m_Batch.getDrawCount(context.frameInFlight);

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = Builtin::PipelineShadow::Point::ResourceAttachment->getRingData()[context.frameInFlight].view,
                    .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
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
                Builtin::PipelineShadow::Point::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineShadow::Point::Resource);

                // TODO multiple objects
                ObjectID object = *m_Scene->getObjects<PointLightComponent>().begin();

                std::uint64_t frustumOffset = m_Culling.getFrustumOffset(object);

                // TODO generate views from a vk::Image
                // for (std::uint32_t cameraId = 0; cameraId < 6; cameraId++) {
                //     Builtin::PipelineShadow::Point::PushConstants pushConstants = {
                //         .frustumOffset = static_cast<std::uint32_t>(frustumOffset),
                //         .frustumId = cameraId,
                //     };
                //
                //     Builtin::PipelineShadow::Point::Resource->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eVertex, std::as_bytes(std::span(&pushConstants, 1)), 0);
                //
                //     // draw
                //     m_Batch.draw(context, m_Culling.getIndirectBuffer(), (frustumOffset + cameraId) * sizeof(vk::DrawIndexedIndirectCommand));
                // }

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
            m_Culling.getVisibleInstanceBuffer(),
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
            m_Culling.getVisibleInstanceBuffer(),
            2);
    }

    return success;
}

bool ShadowRenderer::destroy() {
    bool success = true;

    success &= m_Batch.destroy();
    success &= m_Culling.destroy();

    return success;
}

const RenderGraph &ShadowRenderer::getGraph() const {
    return m_Graph;
}

const ShadowRenderer::Info &ShadowRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
