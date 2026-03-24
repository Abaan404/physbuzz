#include "deferred.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/material.hpp"
#include "components/camera.hpp"
#include "components/lights.hpp"
#include "nodes/camera.hpp"
#include "nodes/lights.hpp"
#include "shadow.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool PipelineDeferred::Geometry::build() {
    if (ResourceRegistry<GraphicsPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<DescriptorLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = DescriptorLayout::Type::eUniformBuffer,
                        .range = sizeof(RenderNodeCamera::CameraBuffer),
                    },
                    {
                        // models
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    success &= ResourceRegistry<GraphicsPipeline>::insert(
        Resource,
        {
            {
                .module = "builtin/render/deferred/geometry",
            },
            {
                .description = &Model::Vertex::Description,
                .blend = {
                    .attachments = {3, {{}}},
                },
                .formats = {
                    .color = {
                        GraphicsPipeline::Format::eR16G16B16A16Sfloat,
                        GraphicsPipeline::Format::eR8G8B8A8Snorm,
                        GraphicsPipeline::Format::eR8G8B8A8Unorm,
                    },
                },
                .layouts = {
                    .resources = {
                        Builtin::LayoutMaterial::Resource,
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
                    .colors = {0, 1, 2},
                },
            },
        });

    return success;
}

bool PipelineDeferred::Lighting::build() {
    if (ResourceRegistry<GraphicsPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<DescriptorLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = DescriptorLayout::Type::eUniformBuffer,
                        .range = sizeof(RenderNodeCamera::CameraBuffer),
                    },
                    {
                        // directionals
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // points
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // spots
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // position
                        .type = DescriptorLayout::Type::eInputAttachment,
                        .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        // normal
                        .type = DescriptorLayout::Type::eInputAttachment,
                        .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        // albedospec
                        .type = DescriptorLayout::Type::eInputAttachment,
                        .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        // directional shadow depth map
                        .type = DescriptorLayout::Type::eCombinedImageSampler,
                        .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        // point shadow depth map
                        .type = DescriptorLayout::Type::eCombinedImageSampler,
                        .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                    },
                },
            }});
    }

    success &= ResourceRegistry<GraphicsPipeline>::insert(
        Resource,
        {
            {
                .module = "builtin/render/deferred/lighting",
                .specialization = {
                    .offsets = {
                        offsetof(Specialization, enableShadows),
                    },
                    .size = sizeof(Specialization),
                },
            },
            {
                .rasterization = {
                    .cullMode = GraphicsPipeline::CullModeFlags::eNone,
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
            },
        });

    return success;
}

bool PipelineDeferred::build() {
    bool success = true;

    if (!ResourceRegistry<Attachment>::contains(ResourceGBuffers[0])) {
        success &= ResourceRegistry<Attachment>::insert(
            ResourceGBuffers[0],
            {{
                .usage = Attachment::Usage::Color,
                .format = Attachment::Format::eR16G16B16A16Sfloat,
            }},
            glm::uvec2{1, 1});
    }

    if (!ResourceRegistry<Attachment>::contains(ResourceGBuffers[1])) {
        success &= ResourceRegistry<Attachment>::insert(
            ResourceGBuffers[1],
            {{
                .usage = Attachment::Usage::Color,
                .format = Attachment::Format::eR8G8B8A8Snorm,
            }},
            glm::uvec2{1, 1});
    }

    if (!ResourceRegistry<Attachment>::contains(ResourceGBuffers[2])) {
        success &= ResourceRegistry<Attachment>::insert(
            ResourceGBuffers[2],
            {{
                .usage = Attachment::Usage::Color,
                .format = Attachment::Format::eR8G8B8A8Unorm,
            }},
            glm::uvec2{1, 1});
    }

    success &= Builtin::PipelineDeferred::Geometry::build();
    success &= Builtin::PipelineDeferred::Lighting::build();

    return success;
}

} // namespace Builtin

DeferredRenderer::DeferredRenderer(const Info &info)
    : m_Info(info) {}

bool DeferredRenderer::build() {
    // build pipeline
    if (!Builtin::PipelineDeferred::build()) {
        Logger::ERROR("[DeferredRenderer] Could not build deferred pipeline.");
        return false;
    }

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([&](const WindowSwapchainResizeEvent &event) {
            const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
            camera.resize(event.resolution);
        }),
    };

    m_State.build(m_Objects);

    m_Graph.add("builtin/camera", Builtin::RenderNodeCamera::build(m_Info.camera));
    m_Graph.add("builtin/lights", Builtin::RenderNodeLights::build());
    m_Graph.merge(m_State.getGraph());

    m_Graph.add(
        "builtin/deferred/gbuffers",
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            Builtin::RenderNodeCamera::ResourceBuffer,
                            {
                                .stage = RenderNode::Stage::Graphics,
                            },
                        },
                        {
                            m_State.getInstanceBuffer(),
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_State.getIndirectBuffer(),
                            {
                                .stage = RenderNode::Stage::Indirect,
                            },
                        },
                    },
                },
                .attachments = {
                    .output = {
                        {
                            Builtin::PipelineDeferred::ResourceGBuffers[0],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::PipelineDeferred::ResourceGBuffers[1],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::PipelineDeferred::ResourceGBuffers[2],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .prepare = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("DeferredRenderer/GBuffer/Prepare");

                std::array gBuffers = {
                    Resource<Attachment>("builtin/deferred/gBuffer0"),
                    Resource<Attachment>("builtin/deferred/gBuffer1"),
                    Resource<Attachment>("builtin/deferred/gBuffer2"),
                };

                for (std::size_t i = 0; i < gBuffers.size(); i++) {
                    glm::uvec2 resolution = gBuffers[i]->getSize(context.frameInFlight);
                    if (resolution.x != context.extent.width || resolution.y != context.extent.height) {
                        gBuffers[i]->rebuild(context, {context.extent.width, context.extent.height});
                    }
                }
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("DeferredRenderer/GBuffer/Execute");
                TracyVkZone(context.tracy, context.command, "DeferredRenderer/GBuffer");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                std::array gBuffers = {
                    Resource<Attachment>("builtin/deferred/gBuffer0"),
                    Resource<Attachment>("builtin/deferred/gBuffer1"),
                    Resource<Attachment>("builtin/deferred/gBuffer2"),
                };

                std::array<vk::RenderingAttachmentInfo, gBuffers.size()> colorAttachments;

                for (std::size_t i = 0; i < gBuffers.size(); i++) {
                    colorAttachments[i] = {
                        .imageView = gBuffers[i]->getRingData()[context.frameInFlight].view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eClear,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
                    };
                }

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = context.depth->getRingData()[context.frameInFlight].view,
                    .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

                // record constants
                Builtin::PipelineDeferred::Geometry::PushConstants pushConstants = {
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getData().address,
                };

                Builtin::PipelineDeferred::Geometry::Resource->updatePushConstants(
                    context,
                    GraphicsPipeline::PushConstantsStageFlags::eAll,
                    std::as_bytes(std::span(&pushConstants, 1)),
                    0);

                context.command.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(context.extent.width), static_cast<float>(context.extent.height), 0.0f, 1.0f});
                context.command.setScissor(0, vk::Rect2D{{0, 0}, context.extent});

                // issue draw calls
                context.command.beginRendering({
                    .renderArea = {
                        .offset = {0, 0},
                        .extent = context.extent,
                    },
                    .layerCount = 1,
                    .colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments.size()),
                    .pColorAttachments = colorAttachments.data(),
                    .pDepthAttachment = &depthAttachment,
                    .pStencilAttachment = {},
                });

                std::array inputIndices = {0u, 1u, 2u};

                context.command.setRenderingInputAttachmentIndices({
                    .colorAttachmentCount = inputIndices.size(),
                    .pColorAttachmentInputIndices = inputIndices.data(),
                });

                // bind resources
                Builtin::PipelineDeferred::Geometry::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineDeferred::Geometry::Resource);

                // draw
                m_State.draw(context);

                context.command.endRendering();
            },
        });

    m_Graph.add(
        Output,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            Builtin::RenderNodeCamera::ResourceBuffer,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::RenderNodeLights::ResourceBufferDirectional,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::RenderNodeLights::ResourceBufferPoint,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::RenderNodeLights::ResourceBufferSpot,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
                .attachments = {
                    .input = {
                        {
                            Builtin::PipelineDeferred::ResourceGBuffers[0],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::PipelineDeferred::ResourceGBuffers[1],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::PipelineDeferred::ResourceGBuffers[2],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::PipelineShadow::Directional::ResourceAttachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
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
                ZoneScopedN("DeferredRenderer/Lighting/Execute");
                TracyVkZone(context.tracy, context.command, "DeferredRenderer/Lighting");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                const std::vector<DirectionalLightComponent> &directionals = scene->getComponentArray<DirectionalLightComponent>();
                const std::vector<PointLightComponent> &points = scene->getComponentArray<PointLightComponent>();
                const std::vector<SpotLightComponent> &spots = scene->getComponentArray<SpotLightComponent>();

                std::array colorAttachments = {
                    vk::RenderingAttachmentInfo{
                        .imageView = context.color.view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eClear,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
                    },
                };

                context.command.beginRendering({
                    .renderArea = {
                        .offset = {0, 0},
                        .extent = context.extent,
                    },
                    .layerCount = 1,
                    .colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments.size()),
                    .pColorAttachments = colorAttachments.data(),
                    .pDepthAttachment = {},
                    .pStencilAttachment = {},
                });

                // bind resources
                Builtin::PipelineDeferred::Lighting::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineDeferred::Lighting::Resource);

                // record constants
                Builtin::PipelineDeferred::Lighting::PushConstants pushConstants = {
                    .directionalCount = static_cast<std::uint32_t>(directionals.size()),
                    .spotCount = static_cast<std::uint32_t>(spots.size()),
                    .pointCount = static_cast<std::uint32_t>(points.size()),
                };

                Builtin::PipelineDeferred::Lighting::Resource->updatePushConstants(
                    context,
                    GraphicsPipeline::PushConstantsStageFlags::eAll,
                    std::as_bytes(std::span(&pushConstants, 1)),
                    0);

                // draw one triangle
                context.command.draw(3, 1, 0, 0);

                context.command.endRendering();
            },
        });

    bool success = true;

    success &= m_Graph.compile();

    if (success) {
        // geometry
        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Geometry::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Geometry::ResourceLayoutFrame,
            m_State.getInstanceBuffer(),
            1);

        // lighting
        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            1);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            2);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferSpot,
            3);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Resource<Attachment>("builtin/deferred/gBuffer0"),
            4);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Resource<Attachment>("builtin/deferred/gBuffer1"),
            5);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Resource<Attachment>("builtin/deferred/gBuffer2"),
            6);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::PipelineShadow::Directional::ResourceAttachment,
            7);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::PipelineShadow::Point::ResourceAttachment,
            8);
    }

    return success;
}

bool DeferredRenderer::destroy() {
    m_Info.window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);
    m_State.destroy();

    return true;
}

bool DeferredRenderer::specialize(const Builtin::PipelineDeferred::Lighting::Specialization &specialization) {
    return Builtin::PipelineDeferred::Lighting::Resource->specialize(specialization);
}

const RenderGraph &DeferredRenderer::getGraph() const {
    return m_Graph;
}

const DeferredRenderer::Info &DeferredRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
