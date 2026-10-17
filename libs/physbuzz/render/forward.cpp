#include "forward.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/pipeline.hpp"
#include "../graphics/renderer.hpp"
#include "components/camera.hpp"
#include "components/lights.hpp"
#include "shadow.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

ForwardRenderer::ForwardRenderer(Scene *scene, const Info &info)
    : System<RenderComponent, TransformComponent>(scene),
      m_Info(info) {}

bool ForwardRenderer::build() {
    Renderer *renderer = m_Scene->getSystem<Renderer>();
    ResourceContext *resourceContext = m_Scene->getSystem<ResourceContext>();

    if (!buildResources(renderer->getMaterial().getResources())) {
        Logger::ERROR("[ForwardRenderer] Could not build forward pipeline.");
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
    success &= m_Irradiance.build();
    success &= m_Prefilter.build();
    success &= m_BrdfLut.build();

    // bake environment textures
    renderer->immediate(m_Irradiance.getGraph());
    renderer->immediate(m_Prefilter.getGraph());
    renderer->immediate(m_BrdfLut.getGraph());

    m_Graph.add("builtin/camera", resourceContext->buildCameraNode());
    m_Graph.add("builtin/lights", resourceContext->buildLightsNode());
    m_Graph.add("builtin/batch", m_Batch.getRenderNode());
    m_Graph.merge(m_Culling.getGraph());

    const ResourceContext::Resources &resources = resourceContext->getResouces();
    const ShadowRenderer::Resources &shadowResources = m_Scene->getSystem<ShadowRenderer>()->getResources();

    m_Graph.add(
        Output,
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
                    .input = {
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
                .textures = {
                    .input = {
                        {
                            m_Irradiance.getIrradianceMap(),
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
                            m_Prefilter.getPrefilterMap(),
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eColor,
                                    .levelCount = Image::RemainingMipLevels,
                                    .layerCount = 1,
                                }},
                            },
                        },
                        {
                            m_BrdfLut.getLut(),
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
            .execute = [this](const RenderContext &context) {
                ZoneScopedN("ForwardRenderer/Execute");
                TracyVkZone(context.tracy, context.command, "ForwardRenderer");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

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

                std::array colorAttachments = {
                    vk::RenderingAttachmentInfo{
                        .imageView = context.color.view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eClear,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
                    },
                };

                const std::vector<DirectionalLightComponent> &directionals = m_Scene->getComponentArray<DirectionalLightComponent>();
                const std::vector<PointLightComponent> &points = m_Scene->getComponentArray<PointLightComponent>();
                const std::vector<SpotLightComponent> &spots = m_Scene->getComponentArray<SpotLightComponent>();

                context.command.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(context.extent.width), static_cast<float>(context.extent.height), 0.0f, 1.0f});
                context.command.setScissor(0, vk::Rect2D{{0, 0}, context.extent});

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

                Resources::PushConstants pushConstants = {
                    .directionalCount = static_cast<std::uint32_t>(directionals.size()),
                    .spotCount = static_cast<std::uint32_t>(spots.size()),
                    .pointCount = static_cast<std::uint32_t>(points.size()),
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getData().address,
                };

                // bind resources
                m_Resources.pipeline->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);
                m_Resources.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.pipeline);

                // draw
                m_Batch.draw(context, m_Culling.getIndirectBuffer());

                context.command.endRendering();
            },
        });

    success &= m_Graph.compile();

    if (success) {
        success &= App::LayoutAllocator.write(
            m_Resources.layoutGlobal,
            m_Irradiance.getIrradianceMap(),
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = Image::RemainingMipLevels,
                    .layerCount = 1,
                },
            },
            0);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutGlobal,
            m_Prefilter.getPrefilterMap(),
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = Image::RemainingMipLevels,
                    .layerCount = 1,
                },
            },
            1);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutGlobal,
            m_BrdfLut.getLut(),
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = Image::RemainingMipLevels,
                    .layerCount = 1,
                },
            },
            2);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            resources.camera,
            0);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            resources.directionalLights,
            1);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            resources.pointLights,
            2);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            resources.spotLights,
            3);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            m_Batch.getInstanceBuffer(),
            4);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            m_Culling.getVisibleInstanceBuffer(),
            5);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            shadowResources.directional.attachment,
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eDepth,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            },
            6);

        success &= App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            shadowResources.point.attachment,
            Image::ViewInfo{
                .type = Image::ViewType::eCube,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eDepth,
                    .levelCount = 1,
                    .layerCount = 6,
                },
            },
            7);
    }

    return success;
}

void ForwardRenderer::tick() {
    m_Renderables.clear();

    for (const auto &object : m_Objects) {
        const auto [render, transform] = m_Scene->getComponent<RenderComponent, TransformComponent>(object);

        m_Renderables.emplace_back<Renderable>({
            .model = &render.model,
            .transform = &transform.transform,
        });
    }
}

bool ForwardRenderer::destroy() {
    m_Info.window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);

    bool success = true;

    success &= m_Batch.destroy();
    success &= m_Culling.destroy();
    success &= m_Irradiance.destroy();
    success &= m_Prefilter.destroy();
    success &= m_BrdfLut.destroy();

    return success;
}

bool ForwardRenderer::specialize(const Resources::Specialization &specialization) {
    return m_Resources.pipeline->specialize(specialization);
}

const RenderGraph &ForwardRenderer::getGraph() const {
    return m_Graph;
}

const ForwardRenderer::Resources &ForwardRenderer::getResources() const {
    return m_Resources;
}

const ForwardRenderer::Info &ForwardRenderer::getInfo() const {
    return m_Info;
}

const RenderBatch &ForwardRenderer::getBatch() const {
    return m_Batch;
}

bool ForwardRenderer::buildResources(const MaterialAllocator::Resources &material) {
    auto layoutGlobal = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/forward/global",
        {{
            .bindings = {
                {
                    // irradiance map
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                },
                {
                    // prefilter map
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                },
                {
                    // brdf lut
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .stage = DescriptorLayout::ShaderStageFlags::eFragment,
                },
            },
            .lifetime = DescriptorLayout::Lifetime::Global,
        }});

    if (!layoutGlobal) {
        return false;
    }

    auto layoutFrame = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/forward/frame",
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
                    // instance
                    .type = DescriptorLayout::Type::eStorageBuffer,
                },
                {
                    // culling
                    .type = DescriptorLayout::Type::eStorageBuffer,
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

    if (!layoutFrame) {
        return false;
    }

    auto pipeline = ResourceRegistry<GraphicsPipeline>::insert(
        "builtin/forward",
        {
            {
                .module = "builtin/render/forward",
                .specialization = {
                    .offsets = {
                        offsetof(Resources::Specialization, enableShadows),
                    },
                    .size = sizeof(Resources::Specialization),
                },
            },
            {
                .description = &Model::Vertex::Description,
                // TODO implement a proper OIT
                .blend = {
                    .logicOpEnable = false,
                    .logicOp = GraphicsPipeline::LogicOp::eCopy,
                    .attachments = {{
                        .blendEnable = true,
                        .srcColorBlendFactor = GraphicsPipeline::BlendFactor::eSrcAlpha,
                        .dstColorBlendFactor = GraphicsPipeline::BlendFactor::eOneMinusSrcAlpha,
                        .colorBlendOp = GraphicsPipeline::BlendOp::eAdd,
                        .srcAlphaBlendFactor = GraphicsPipeline::BlendFactor::eOne,
                        .dstAlphaBlendFactor = GraphicsPipeline::BlendFactor::eZero,
                        .alphaBlendOp = GraphicsPipeline::BlendOp::eAdd,
                        .colorWriteMask = GraphicsPipeline::ColorComponentFlags::eR | //
                                          GraphicsPipeline::ColorComponentFlags::eG | //
                                          GraphicsPipeline::ColorComponentFlags::eB | //
                                          GraphicsPipeline::ColorComponentFlags::eA,
                    }},
                },
                .layouts = {
                    .resources = {
                        material.layout,
                        *layoutGlobal,
                        *layoutFrame,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eAll,
                            .size = sizeof(Resources::PushConstants),
                        },
                    },
                },
            },
        });

    if (!pipeline) {
        return false;
    }

    m_Resources = {
        .layoutGlobal = std::move(*layoutGlobal),
        .layoutFrame = std::move(*layoutFrame),
        .pipeline = std::move(*pipeline),
    };

    return true;
}

} // namespace Physbuzz
