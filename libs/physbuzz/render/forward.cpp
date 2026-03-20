#include "forward.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/material.hpp"
#include "../graphics/pipeline.hpp"
#include "components/camera.hpp"
#include "components/lights.hpp"
#include "nodes/camera.hpp"
#include "nodes/lights.hpp"
#include "shadow.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool PipelineForward::build() {
    if (ResourceRegistry<GraphicsPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<DescriptorLayout>::contains(LayoutMaterial::Resource)) {
        success &= LayoutMaterial::build();
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = DescriptorLayout::Type::eUniformBuffer,
                        .range = sizeof(Builtin::RenderNodeCamera::CameraBuffer),
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
                .module = "builtin/render/forward",
                .specialization = {
                    .offsets = {
                        offsetof(Specialization, enableShadows),
                    },
                    .size = sizeof(Specialization),
                },
            },
            {
                .description = &Model::Vertex::Description,
                .layouts = {
                    .resources = {
                        LayoutMaterial::Resource,
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

} // namespace Builtin

ForwardRenderer::ForwardRenderer(const Info &info)
    : m_Info(info) {}

bool ForwardRenderer::build() {
    // build pipeline
    if (!Builtin::PipelineForward::build()) {
        Logger::ERROR("[ForwardRenderer] Could not build forward pipeline.");
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
    m_Graph.add("builtin/models", m_State.getRenderNode());

    m_Graph.add(
        Output,
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
                    },
                },
                .attachments = {
                    .input = {
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
                ZoneScopedN("ForwardRenderer/Execute");
                TracyVkZone(context.tracy, context.command, "ForwardRenderer");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = context.depth->getRingData()[context.frameInFlight].view,
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

                const std::vector<DirectionalLightComponent> &directionals = scene->getComponentArray<DirectionalLightComponent>();
                const std::vector<PointLightComponent> &points = scene->getComponentArray<PointLightComponent>();
                const std::vector<SpotLightComponent> &spots = scene->getComponentArray<SpotLightComponent>();

                Builtin::PipelineForward::PushConstants pushConstants = {
                    .directionalCount = static_cast<std::uint32_t>(directionals.size()),
                    .spotCount = static_cast<std::uint32_t>(spots.size()),
                    .pointCount = static_cast<std::uint32_t>(points.size()),
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getData().address,
                };

                Builtin::PipelineForward::Resource->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

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

                // bind resources
                Builtin::PipelineForward::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineForward::Resource);

                // draw
                m_State.draw(context);

                context.command.endRendering();
            },
        });

    bool success = true;

    success &= m_Graph.compile();

    if (success) {
        success &= App::LayoutAllocator.write(
            Builtin::PipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            1);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            2);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferSpot,
            3);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineForward::ResourceLayoutFrame,
            Resource<DynamicBuffer>{m_State.getInfo().instanceBufferId},
            4);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineForward::ResourceLayoutFrame,
            Builtin::PipelineShadow::Directional::ResourceAttachment,
            5);

        success &= App::LayoutAllocator.write(
            Builtin::PipelineForward::ResourceLayoutFrame,
            Builtin::PipelineShadow::Point::ResourceAttachment,
            6);
    }

    return success;
}

bool ForwardRenderer::destroy() {
    m_Info.window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);

    m_State.destroy();

    return true;
}

bool ForwardRenderer::specialize(const Builtin::PipelineForward::Specialization &specialization) {
    return Builtin::PipelineForward::Resource->specialize(specialization);
}

const RenderGraph &ForwardRenderer::getGraph() const {
    return m_Graph;
}

const ForwardRenderer::Info &ForwardRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
