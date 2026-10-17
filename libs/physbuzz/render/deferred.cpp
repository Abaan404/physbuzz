#include "deferred.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/material.hpp"
#include "../graphics/renderer.hpp"
#include "components/lights.hpp"
#include "shadow.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

DeferredRenderer::DeferredRenderer(Scene *scene, const Info &info)
    : System<RenderComponent, TransformComponent>(scene),
      m_Info(info) {}

bool DeferredRenderer::build() {
    // build pipeline
    Renderer *renderer = m_Scene->getSystem<Renderer>();
    ResourceContext *resourceContext = m_Scene->getSystem<ResourceContext>();

    const MaterialAllocator::Resources &materialResources = renderer->getMaterial().getResources();

    if (!buildResources(materialResources)) {
        return false;
    }

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([this](const WindowSwapchainResizeEvent &event) {
            ResourceContext *resourceContext = m_Scene->getSystem<ResourceContext>();

            const auto [camera] = m_Scene->getComponent<CameraComponent>(resourceContext->getInfo().cameraId);
            camera.resize(event.resolution);
        }),
    };

    m_Culling.setObjects({
        .cameras = {resourceContext->getInfo().cameraId},
    });

    bool success = true;

    success &= m_Batch.build(m_Renderables);
    success &= m_Culling.build(*m_Scene);

    m_Graph.add("builtin/camera", resourceContext->buildCameraNode());
    m_Graph.add("builtin/lights", resourceContext->buildLightsNode());
    m_Graph.add("builtin/batch", m_Batch.getRenderNode());
    m_Graph.merge(m_Culling.getGraph());

    const ShadowRenderer::Resources &shadowResources = m_Scene->getSystem<ShadowRenderer>()->getResources();
    const ResourceContext::Resources &resources = resourceContext->getResouces();

    m_Graph.add(
        "builtin/deferred/gbuffers",
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
                            resources.camera,
                            {
                                .stage = RenderNode::Stage::Graphics,
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
                            m_Resources.gbuffers[0],
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eColor,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                        {
                            m_Resources.gbuffers[1],
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eColor,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                        {
                            m_Resources.gbuffers[2],
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eColor,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                    },
                },
            },
            .prepare = [this](const RenderContext &context) {
                ZoneScopedN("DeferredRenderer/GBuffer/Prepare");

                for (const auto &gbuffer : m_Resources.gbuffers) {
                    glm::uvec2 resolution = gbuffer->getSize(context.frameInFlight);

                    if (resolution.x != context.extent.width ||
                        resolution.y != context.extent.height) {
                        gbuffer->rebuild(context, {context.extent.width, context.extent.height});
                    }
                }
            },
            .execute = [this](const RenderContext &context) {
                ZoneScopedN("DeferredRenderer/GBuffer/Execute");
                TracyVkZone(context.tracy, context.command, "DeferredRenderer/GBuffer");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                // std::array gBuffers = {
                //     Resource<Attachment>("builtin/deferred/gBuffer0"),
                //     Resource<Attachment>("builtin/deferred/gBuffer1"),
                //     Resource<Attachment>("builtin/deferred/gBuffer2"),
                // };

                std::vector<vk::RenderingAttachmentInfo> colorAttachments(m_Resources.gbuffers.size());

                for (std::size_t i = 0; i < m_Resources.gbuffers.size(); i++) {
                    const Image::Data &data = m_Resources.gbuffers[i]->getRingData()[context.frameInFlight].image.getData();

                    colorAttachments[i] = {
                        .imageView = data.views.at({
                            .type = Image::ViewType::e2D,
                            .subresourceRange = {
                                .aspectMask = Image::AspectFlags::eColor,
                                .levelCount = 1,
                                .layerCount = 1,
                            },
                        }),
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eClear,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
                    };
                }

                const Image::Data &data = context.depth->getRingData()[context.frameInFlight].image.getData();

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = data.views.at({
                        .type = Image::ViewType::e2D,
                        .subresourceRange = {
                            .aspectMask = Image::AspectFlags::eDepth | Image::AspectFlags::eStencil,
                            .levelCount = 1,
                            .layerCount = 1,
                        },
                    }),
                    .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

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

                Resources::Geometry::PushConstants pushConstants = {
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getData().address,
                };

                // bind resources
                m_Resources.geometry.pipeline->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);
                m_Resources.geometry.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.geometry.pipeline);

                // draw
                m_Batch.draw(context, m_Culling.getIndirectBuffer());

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
                            resources.camera,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            resources.directionalLights,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            resources.pointLights,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            resources.spotLights,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
                .attachments = {
                    .input = {
                        {
                            m_Resources.gbuffers[0],
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eColor,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                        {
                            m_Resources.gbuffers[1],
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eColor,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                        {
                            m_Resources.gbuffers[2],
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eColor,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                        {
                            shadowResources.directional.attachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eDepth,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                        {
                            shadowResources.point.attachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eDepth,
                                    .levelCount = 1,
                                    .layerCount = 6,
                                }},
                            },
                        },
                    },
                },
            },
            .execute = [this](const RenderContext &context) {
                ZoneScopedN("DeferredRenderer/Lighting/Execute");
                TracyVkZone(context.tracy, context.command, "DeferredRenderer/Lighting");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                const std::vector<DirectionalLightComponent> &directionals = m_Scene->getComponentArray<DirectionalLightComponent>();
                const std::vector<PointLightComponent> &points = m_Scene->getComponentArray<PointLightComponent>();
                const std::vector<SpotLightComponent> &spots = m_Scene->getComponentArray<SpotLightComponent>();

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

                Resources::Lighting::PushConstants pushConstants = {
                    .directionalCount = static_cast<std::uint32_t>(directionals.size()),
                    .spotCount = static_cast<std::uint32_t>(spots.size()),
                    .pointCount = static_cast<std::uint32_t>(points.size()),
                };

                // bind resources
                m_Resources.lighting.pipeline->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);
                m_Resources.lighting.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.lighting.pipeline);

                // draw one triangle
                context.command.draw(3, 1, 0, 0);

                context.command.endRendering();
            },
        });

    success &= m_Graph.compile();

    if (success) {
        // geometry
        success &= App::LayoutAllocator.write(
            m_Resources.geometry.layout,
            resources.camera,
            0);

        success &= App::LayoutAllocator.write(
            m_Resources.geometry.layout,
            m_Batch.getInstanceBuffer(),
            1);

        success &= App::LayoutAllocator.write(
            m_Resources.geometry.layout,
            m_Culling.getVisibleInstanceBuffer(),
            2);

        // lighting
        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            resources.camera,
            0);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            resources.directionalLights,
            1);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            resources.pointLights,
            2);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            resources.spotLights,
            3);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            m_Resources.gbuffers[0],
            {
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            },
            4);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            m_Resources.gbuffers[1],
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            },
            5);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            m_Resources.gbuffers[2],
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            },
            6);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            shadowResources.directional.attachment,
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eDepth,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            },
            7);

        success &= App::LayoutAllocator.write(
            m_Resources.lighting.layout,
            shadowResources.point.attachment,
            Image::ViewInfo{
                .type = Image::ViewType::eCube,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eDepth,
                    .levelCount = 1,
                    .layerCount = 6,
                },
            },
            8);
    }

    return success;
}

void DeferredRenderer::tick() {
    m_Renderables.clear();

    for (const auto &object : m_Objects) {
        const auto [render, transform] = m_Scene->getComponent<RenderComponent, TransformComponent>(object);

        m_Renderables.emplace_back<Renderable>({
            .model = &render.model,
            .transform = &transform.transform,
        });
    }
}

bool DeferredRenderer::destroy() {
    m_Info.window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);

    m_Batch.destroy();
    m_Culling.destroy();

    return true;
}

bool DeferredRenderer::specialize(const Resources::Lighting::Specialization &specialization) {
    return m_Resources.lighting.pipeline->specialize(specialization);
}

const RenderBatch &DeferredRenderer::getBatch() const {
    return m_Batch;
}

const RenderGraph &DeferredRenderer::getGraph() const {
    return m_Graph;
}

const DeferredRenderer::Resources &DeferredRenderer::getResources() const {
    return m_Resources;
}

const DeferredRenderer::Info &DeferredRenderer::getInfo() const {
    return m_Info;
}

bool DeferredRenderer::buildResources(const MaterialAllocator::Resources &material) {
    auto gbuffer0 = ResourceRegistry<Attachment>::insert(
        "builtin/deferred/gBuffer0",
        {{
            .usage = Attachment::Usage::Color,
            .format = Attachment::Format::eR16G16B16A16Sfloat,
            .views = {
                {
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .levelCount = 1,
                        .layerCount = 1,
                    },
                },
            },
        }},
        glm::uvec2{1, 1});

    if (!gbuffer0) {
        return false;
    }

    auto gbuffer1 = ResourceRegistry<Attachment>::insert(
        "builtin/deferred/gBuffer1",
        {{
            .usage = Attachment::Usage::Color,
            .format = Attachment::Format::eR8G8B8A8Snorm,
            .views = {
                {
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .levelCount = 1,
                        .layerCount = 1,
                    },
                },
            },
        }},
        glm::uvec2{1, 1});

    if (!gbuffer1) {
        return false;
    }

    auto gbuffer2 = ResourceRegistry<Attachment>::insert(
        "builtin/deferred/gBuffer2",
        {{
            .usage = Attachment::Usage::Color,
            .format = Attachment::Format::eR8G8B8A8Unorm,
            .views = {
                {
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .levelCount = 1,
                        .layerCount = 1,
                    },
                },
            },
        }},
        glm::uvec2{1, 1});

    if (!gbuffer2) {
        return false;
    }

    auto geometry = buildGeometryResources(material);
    if (!geometry) {
        return false;
    }

    auto lighting = buildLightingResources();
    if (!lighting) {
        return false;
    }

    m_Resources = {
        .gbuffers = {std::move(*gbuffer0), std::move(*gbuffer1), std::move(*gbuffer2)},
        .geometry = std::move(*geometry),
        .lighting = std::move(*lighting),
    };

    return true;
}

std::optional<DeferredRenderer::Resources::Geometry> DeferredRenderer::buildGeometryResources(const MaterialAllocator::Resources &material) {
    auto layout = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/deferred/geometry/frame",
        {{
            .bindings = {
                {
                    // camera
                    .type = DescriptorLayout::Type::eUniformBuffer,
                    .range = sizeof(ResourceContext::CameraBufferData),
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

    if (!layout) {
        return std::nullopt;
    }

    auto pipeline = ResourceRegistry<GraphicsPipeline>::insert(
        "builtin/deferred/geometry",
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
                        material.layout,
                        *layout,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eAll,
                            .size = sizeof(Resources::Geometry::PushConstants),
                        },
                    },
                },
                .inputs = {
                    .colors = {0, 1, 2},
                },
            },
        });

    if (!pipeline) {
        return std::nullopt;
    }

    return std::make_optional<Resources::Geometry>({
        .layout = std::move(*layout),
        .pipeline = std::move(*pipeline),
    });
}

std::optional<DeferredRenderer::Resources::Lighting> DeferredRenderer::buildLightingResources() {
    auto layout = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/deferred/lighting/frame",
        {{
            .bindings = {
                {
                    // camera
                    .type = DescriptorLayout::Type::eUniformBuffer,
                    .range = sizeof(ResourceContext::CameraBufferData),
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

    if (!layout) {
        return std::nullopt;
    }

    auto pipeline = ResourceRegistry<GraphicsPipeline>::insert(
        "builtin/deferred/lighting",
        {
            {
                .module = "builtin/render/deferred/lighting",
                .specialization = {
                    .offsets = {
                        offsetof(Resources::Lighting::Specialization, enableShadows),
                    },
                    .size = sizeof(Resources::Lighting::Specialization),
                },
            },
            {
                .rasterization = {
                    .cullMode = GraphicsPipeline::CullModeFlags::eNone,
                },
                .layouts = {
                    .resources = {
                        *layout,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eAll,
                            .size = sizeof(Resources::Lighting::PushConstants),
                        },
                    },
                },
            },
        });

    if (!layout) {
        return std::nullopt;
    }

    return std::make_optional<Resources::Lighting>({
        .layout = std::move(*layout),
        .pipeline = std::move(*pipeline),
    });
}

} // namespace Physbuzz
