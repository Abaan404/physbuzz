#include "shadow.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../graphics/layout.hpp"
#include "nodes/lights.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineShadow::Directional::build(const glm::uvec2 &resolution) {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
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

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // lights
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                    {
                        // instance
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/render/shadow/directional",
            .description = &Model::Vertex::Description,
            .rasterization = {
                .cullMode = vk::CullModeFlagBits::eFront,
            },
            .blend = {
                .attachments = {{}},
            },
            .formats = {
                .color = {},
                .depth = RenderPipeline::Format::eD32Sfloat,
            },
            .layouts = {
                .resources = {
                    ResourceLayoutFrame,
                },
            },
            .inputs = {
                .colors = {},
            },
        }});

    return success;
}

bool RenderPipelineShadow::Point::build(const glm::uvec2 &resolution) {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
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

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // lights
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                    {
                        // instance
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/render/shadow/point",
            .description = &Model::Vertex::Description,
            .rasterization = {
                .cullMode = vk::CullModeFlagBits::eFront,
            },
            .blend = {
                .attachments = {{}},
            },
            .formats = {
                .color = {},
                .depth = RenderPipeline::Format::eD32Sfloat,
                .viewMask = 0x3F,
            },
            .layouts = {
                .resources = {
                    ResourceLayoutFrame,
                },
            },
            .inputs = {
                .colors = {},
            },
        }});

    return success;
}

} // namespace Builtin

ShadowRenderer::ShadowRenderer(const Info &info)
    : m_Info(info) {}

bool ShadowRenderer::build() {
    bool success = true;

    success &= Builtin::RenderPipelineShadow::Directional::build(m_Info.resolution);
    success &= Builtin::RenderPipelineShadow::Point::build(m_Info.resolution);

    m_State.build(m_Objects);

    m_Graph.add("builtin/lights", Builtin::RenderNodeLights::build());
    m_Graph.add("builtin/models", m_State.getRenderNode());

    m_Graph.add(
        Output2D,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_State.getInfo().instanceBufferId,
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_State.getInfo().indirectBufferId,
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
                    },
                },
                .attachments = {
                    .output = {
                        {
                            Builtin::RenderPipelineShadow::Directional::ResourceAttachment,
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

                std::lock_guard<std::mutex> lock(ResourceRegistry<RenderPipeline>::ReloadMutex);

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = Builtin::RenderPipelineShadow::Directional::ResourceAttachment->getRingData()[context.frameInFlight].view,
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
                Builtin::RenderPipelineShadow::Directional::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::RenderPipelineShadow::Directional::Resource);

                // draw
                m_State.draw(context);

                context.command.endRendering();
            },
        });

    m_Graph.add(
        OutputCube,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_State.getInfo().instanceBufferId,
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_State.getInfo().indirectBufferId,
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
                    },
                },
                .attachments = {
                    .output = {
                        {
                            Builtin::RenderPipelineShadow::Point::ResourceAttachment,
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

                std::lock_guard<std::mutex> lock(ResourceRegistry<RenderPipeline>::ReloadMutex);

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = Builtin::RenderPipelineShadow::Point::ResourceAttachment->getRingData()[context.frameInFlight].view,
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
                    .viewMask = 0x3F,
                    .pDepthAttachment = &depthAttachment,
                    .pStencilAttachment = {},
                });

                // bind resources
                Builtin::RenderPipelineShadow::Point::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::RenderPipelineShadow::Point::Resource);

                // draw
                m_State.draw(context);

                context.command.endRendering();
            },
        });

    if (success) {
        App::LayoutAllocator.write(
            Builtin::RenderPipelineShadow::Directional::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            0);

        App::LayoutAllocator.write(
            Builtin::RenderPipelineShadow::Directional::ResourceLayoutFrame,
            Resource<DynamicBuffer>{m_State.getInfo().instanceBufferId},
            1);

        App::LayoutAllocator.write(
            Builtin::RenderPipelineShadow::Point::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            0);

        App::LayoutAllocator.write(
            Builtin::RenderPipelineShadow::Point::ResourceLayoutFrame,
            Resource<DynamicBuffer>{m_State.getInfo().instanceBufferId},
            1);
    }

    return success;
}

bool ShadowRenderer::destroy() {
    return true;
}

const RenderGraph &ShadowRenderer::getGraph() const {
    return m_Graph;
}

const ShadowRenderer::Info &ShadowRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
