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
#include "nodes/models.hpp"
#include "shadow.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineForward::build() {
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

    if (!ResourceRegistry<PipelineLayout>::contains(LayoutMaterial::Resource)) {
        success &= LayoutMaterial::build();
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(Builtin::RenderNodeCamera::CameraBuffer),
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
                        // instance
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                    {
                        // directional shadow depth map
                        .type = PipelineLayout::Type::eCombinedImageSampler,
                        .stage = PipelineLayout::ShaderStageFlags::eFragment,
                    },
                },
            }});
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/forward",
            .description = &Model::Vertex::Description,
            .layouts = {
                .resources = {
                    LayoutMaterial::Resource,
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

} // namespace Builtin

ForwardRenderer::ForwardRenderer(const Info &info)
    : m_Info(info) {}

bool ForwardRenderer::build() {
    // build pipeline
    if (!Builtin::RenderPipelineForward::build()) {
        Logger::ERROR("[ForwardRenderer] Could not build forward pipeline.");
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
    m_Graph.add("builtin/models", Builtin::RenderNodeModels::build(Builtin::RenderPipelineForward::ResourceModel, m_Objects, m_Batches));

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
                            Builtin::RenderPipelineForward::ResourceModel,
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                    },
                },
                .attachments = {
                    .input = {
                        {

                            Builtin::RenderPipelineShadowDirectional::ResourceAttachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = context.depth->getRingData()[context.frameInFlight].view,
                    .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

                std::vector<vk::RenderingAttachmentInfo> colorAttachments = {
                    {
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

                Builtin::RenderPipelineForward::PushConstants pushConstants = {
                    .directionalCount = static_cast<std::uint32_t>(directionals.size()),
                    .spotCount = static_cast<std::uint32_t>(spots.size()),
                    .pointCount = static_cast<std::uint32_t>(points.size()),
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getData().address,
                };

                Builtin::RenderPipelineForward::Resource->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

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
                Builtin::RenderPipelineForward::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::RenderPipelineForward::Resource);

                // draw
                std::uint32_t object = 0;
                for (const auto &[mesh, batch] : m_Batches) {
                    if (mesh->getDescription() != Builtin::RenderPipelineForward::Resource->getInfo().description) {
                        Logger::ERROR("[ForwardRenderer] Incompatible vertex state descriptions.");
                        continue;
                    }

                    mesh->draw(context, batch, object);
                    object += batch;
                }

                context.command.endRendering();
            },
        });

    bool success = true;

    success &= m_Graph.compile();

    if (success) {
        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            1);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            2);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferSpot,
            3);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderPipelineForward::ResourceModel,
            4);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderPipelineShadowDirectional::ResourceAttachment,
            5);
    }

    return success;
}

bool ForwardRenderer::destroy() {
    m_Info.window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);
    return true;
}

const RenderGraph &ForwardRenderer::getGraph() const {
    return m_Graph;
}

const ForwardRenderer::Info &ForwardRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
