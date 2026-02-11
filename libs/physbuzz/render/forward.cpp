#include "forward.hpp"

#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/material.hpp"
#include "../graphics/pipeline.hpp"
#include "camera.hpp"
#include "lighting.hpp"
#include "nodes/camera.hpp"
#include "nodes/lights.hpp"
#include "nodes/models.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineForward::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

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
    if (m_Info.pipeline == Builtin::RenderPipelineForward::Resource) {
        if (!Builtin::RenderPipelineForward::build()) {
            Logger::ERROR("[ForwardRenderer] Could not build forward shader pipeline.");
            return false;
        }
    }

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([&](const WindowSwapchainResizeEvent &event) {
            const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
            camera.resize(event.resolution);
        }),
    };

    m_Graph.add(Builtin::RenderNodeCamera::Id, Builtin::RenderNodeCamera::build(m_Info.camera));
    m_Graph.add(Builtin::RenderNodeLights::Id, Builtin::RenderNodeLights::build());
    m_Graph.add(Builtin::RenderNodeModels::Id, Builtin::RenderNodeModels::build(m_Objects, m_Batches));

    m_Graph.add(
        "builtin/forward",
        {
            .description = {
                .buffers = {
                    .input = {
                        Builtin::RenderNodeCamera::ResourceBuffer,
                        Builtin::RenderNodeLights::ResourceBufferDirectional,
                        Builtin::RenderNodeLights::ResourceBufferPoint,
                        Builtin::RenderNodeLights::ResourceBufferSpot,
                        Builtin::RenderNodeModels::ResourceBuffer,
                    },
                },
            },
            .execute = [&](Scene *scene, const RenderContext &context) {
                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = context.depth.view,
                    .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eDontCare,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

                std::vector<vk::RenderingAttachmentInfo> colorAttachments = {
                    {
                        .imageView = context.color.view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eLoad,
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
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getAddress(),
                };

                m_Info.pipeline->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

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
                m_Info.pipeline->bind(context);
                context.systems.allocator->bind(context, m_Info.pipeline);

                // draw
                std::uint32_t object = 0;
                for (const auto &[mesh, batch] : m_Batches) {
                    if (mesh->getDescription() != m_Info.pipeline->getInfo().description) {
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
        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            1);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            2);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferSpot,
            3);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderNodeModels::ResourceBuffer,
            4);
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
