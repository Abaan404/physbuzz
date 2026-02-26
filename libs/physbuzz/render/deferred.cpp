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
#include "nodes/models.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineDeferred::Geometry::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceModel)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceModel,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(RenderNodeModels::ModelBuffer));
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(RenderNodeCamera::CameraBuffer),
                    },
                    {
                        // models
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/deferred/geometry",
            .description = &Model::Vertex::Description,
            .blend = {
                .attachments = {3, {{}}},
            },
            .formats = {
                .color = {
                    RenderPipeline::Format::eR16G16B16A16Sfloat,
                    RenderPipeline::Format::eR8G8B8A8Snorm,
                    RenderPipeline::Format::eR8G8B8A8Unorm,
                },
            },
            .layouts = {
                .resources = {
                    Builtin::LayoutMaterial::Resource,
                    ResourceLayoutFrame,
                },
                .pushConstantRanges = {
                    {
                        .stageFlags = RenderPipeline::PushConstantsStageFlags::eAll,
                        .size = sizeof(PushConstants),
                    },
                },
            },
            .attachments = {
                .colors = {0, 1, 2},
            },
        }});

    return success;
}

bool RenderPipelineDeferred::Lighting::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(RenderNodeCamera::CameraBuffer),
                    },
                    {
                        // directionals
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                    {
                        // points
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                    {
                        // spots
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                    {
                        // position
                        .type = PipelineLayout::Type::eInputAttachment,
                        .stage = PipelineLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        // normal
                        .type = PipelineLayout::Type::eInputAttachment,
                        .stage = PipelineLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        // albedospec
                        .type = PipelineLayout::Type::eInputAttachment,
                        .stage = PipelineLayout::ShaderStageFlags::eFragment,
                    },
                },
            }});
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/deferred/lighting",
            .rasterization = {
                .cullMode = RenderPipeline::CullModeFlags::eNone,
            },
            .layouts = {
                .resources = {
                    ResourceLayoutFrame,
                },
                .pushConstantRanges = {
                    {
                        .stageFlags = RenderPipeline::PushConstantsStageFlags::eAll,
                        .size = sizeof(PushConstants),
                    },
                },
            },
        }});

    return success;
}

bool RenderPipelineDeferred::build() {
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

    success &= Builtin::RenderPipelineDeferred::Geometry::build();
    success &= Builtin::RenderPipelineDeferred::Lighting::build();

    return success;
}

} // namespace Builtin

DeferredRenderer::DeferredRenderer(const Info &info)
    : m_Info(info) {}

bool DeferredRenderer::build() {
    // build pipeline
    if (!Builtin::RenderPipelineDeferred::build()) {
        Logger::ERROR("[DeferredRenderer] Could not build deferred pipeline.");
        return false;
    }

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([&](const WindowSwapchainResizeEvent &event) {
            const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
            camera.resize(event.resolution);
        }),
    };

    m_Graph.add("builtin/camera", Builtin::RenderNodeCamera::build(m_Info.camera));
    m_Graph.add("builtin/lights", Builtin::RenderNodeLights::build());
    m_Graph.add("builtin/models", Builtin::RenderNodeModels::build(Builtin::RenderPipelineDeferred::Geometry::ResourceModel, m_Objects, m_Batches));

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
                            Builtin::RenderPipelineDeferred::Geometry::ResourceModel,
                            {
                                .stage = RenderNode::Stage::Graphics,
                            },
                        },
                    },
                },
                .attachments = {
                    .output = {
                        {
                            Builtin::RenderPipelineDeferred::ResourceGBuffers[0],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::RenderPipelineDeferred::ResourceGBuffers[1],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::RenderPipelineDeferred::ResourceGBuffers[2],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .prepare = [this](Scene *scene, const RenderContext &context) {
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
                Builtin::RenderPipelineDeferred::Geometry::PushConstants pushConstants = {
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getData().address,
                };

                Builtin::RenderPipelineDeferred::Geometry::Resource->updatePushConstants(
                    context,
                    RenderPipeline::PushConstantsStageFlags::eAll,
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
                Builtin::RenderPipelineDeferred::Geometry::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::RenderPipelineDeferred::Geometry::Resource);

                std::uint32_t object = 0;
                for (const auto &[mesh, batch] : m_Batches) {
                    if (mesh->getDescription() != Builtin::RenderPipelineDeferred::Geometry::Resource->getInfo().description) {
                        Logger::ERROR("[DeferredRenderer] Incompatible vertex state descriptions.");
                        continue;
                    }

                    mesh->draw(context, batch, object);
                    object += batch;
                }

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
                            Builtin::RenderPipelineDeferred::ResourceGBuffers[0],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::RenderPipelineDeferred::ResourceGBuffers[1],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            Builtin::RenderPipelineDeferred::ResourceGBuffers[2],
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                const std::vector<DirectionalLightComponent> &directionals = scene->getComponentArray<DirectionalLightComponent>();
                const std::vector<PointLightComponent> &points = scene->getComponentArray<PointLightComponent>();
                const std::vector<SpotLightComponent> &spots = scene->getComponentArray<SpotLightComponent>();

                std::vector<vk::RenderingAttachmentInfo> colorAttachments = {
                    {
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
                Builtin::RenderPipelineDeferred::Lighting::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::RenderPipelineDeferred::Lighting::Resource);

                // record constants
                Builtin::RenderPipelineDeferred::Lighting::PushConstants pushConstants = {
                    .directionalCount = static_cast<std::uint32_t>(directionals.size()),
                    .spotCount = static_cast<std::uint32_t>(spots.size()),
                    .pointCount = static_cast<std::uint32_t>(points.size()),
                };

                Builtin::RenderPipelineDeferred::Lighting::Resource->updatePushConstants(
                    context,
                    RenderPipeline::PushConstantsStageFlags::eAll,
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
            Builtin::RenderPipelineDeferred::Geometry::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Geometry::ResourceLayoutFrame,
            Builtin::RenderPipelineDeferred::Geometry::ResourceModel,
            1);

        // lighting
        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            1);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            2);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferSpot,
            3);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Resource<Attachment>("builtin/deferred/gBuffer0"),
            4);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Resource<Attachment>("builtin/deferred/gBuffer1"),
            5);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Resource<Attachment>("builtin/deferred/gBuffer2"),
            6);
    }

    return success;
}

const RenderGraph &DeferredRenderer::getGraph() const {
    return m_Graph;
}

bool DeferredRenderer::destroy() {
    m_Info.window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);

    return true;
}

const DeferredRenderer::Info &DeferredRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
