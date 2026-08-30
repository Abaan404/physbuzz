#include "collision_highlight.hpp"
#include "physbuzz/app/application.hpp"
#include "physbuzz/ecs/scene.hpp"
#include "physbuzz/events/window.hpp"
#include "physbuzz/render/components/camera.hpp"

#include <physbuzz/graphics/layout.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/graphics/pipeline.hpp>
#include <physbuzz/render/nodes/camera.hpp>
#include <tracy/Tracy.hpp>

bool PipelineCollisionHighlight::build() {
    if (Physbuzz::ResourceRegistry<Physbuzz::GraphicsPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!Physbuzz::ResourceRegistry<Physbuzz::DynamicBuffer>::contains(ResourceHighlightBuffer)) {
        success &= Physbuzz::ResourceRegistry<Physbuzz::DynamicBuffer>::insert(
            ResourceHighlightBuffer,
            {{
                .type = Physbuzz::DynamicBuffer::Type::Structured,
            }},
            sizeof(HighlightedObject));
    }

    if (!Physbuzz::ResourceRegistry<Physbuzz::DescriptorLayout>::contains(ResourceLayoutFrame)) {
        success &= Physbuzz::ResourceRegistry<Physbuzz::DescriptorLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = Physbuzz::DescriptorLayout::Type::eUniformBuffer,
                        .range = sizeof(Physbuzz::Builtin::RenderNodeCamera::CameraBuffer),
                    },
                    {
                        // highlighted
                        .type = Physbuzz::DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // objects
                        .type = Physbuzz::DescriptorLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    success &= Physbuzz::ResourceRegistry<Physbuzz::GraphicsPipeline>::insert(
        Resource,
        {
            {
                .module = "collision_highlight",
            },
            {
                .description = &Physbuzz::Model::Vertex::Description,
                .blend = {
                    .logicOpEnable = false,
                    .logicOp = Physbuzz::GraphicsPipeline::LogicOp::eCopy,
                    .attachments = {{
                        .blendEnable = true,
                        .srcColorBlendFactor = Physbuzz::GraphicsPipeline::BlendFactor::eSrcAlpha,
                        .dstColorBlendFactor = Physbuzz::GraphicsPipeline::BlendFactor::eOneMinusSrcAlpha,
                        .colorBlendOp = Physbuzz::GraphicsPipeline::BlendOp::eAdd,
                        .srcAlphaBlendFactor = Physbuzz::GraphicsPipeline::BlendFactor::eZero,
                        .dstAlphaBlendFactor = Physbuzz::GraphicsPipeline::BlendFactor::eOne,
                        .alphaBlendOp = Physbuzz::GraphicsPipeline::BlendOp::eAdd,
                        .colorWriteMask = Physbuzz::GraphicsPipeline::ColorComponentFlags::eR | //
                                          Physbuzz::GraphicsPipeline::ColorComponentFlags::eG | //
                                          Physbuzz::GraphicsPipeline::ColorComponentFlags::eB | //
                                          Physbuzz::GraphicsPipeline::ColorComponentFlags::eA,
                    }},
                },
                .layouts = {
                    .resources = {
                        ResourceLayoutFrame,
                    },
                },
            },
        });

    return success;
}

CollisionHighlight::CollisionHighlight(Physbuzz::Scene *scene, const Info &info)
    : Physbuzz::System<>(scene),
      m_Info(info) {}

bool CollisionHighlight::build() {
    // build pipeline
    if (!PipelineCollisionHighlight::build()) {
        Physbuzz::Logger::ERROR("[CollisionHighlight] Could not build collision highlight pipeline.");
        return false;
    }

    m_Events = {
        .resize = m_Info.window->addCallback<Physbuzz::WindowSwapchainResizeEvent>([&](const Physbuzz::WindowSwapchainResizeEvent &event) {
            const auto [camera] = m_Scene->getComponent<Physbuzz::CameraComponent>(m_Info.camera);
            camera.resize(event.resolution);
        }),
    };

    bool success = true;

    m_Graph.add("builtin/camera", Physbuzz::Builtin::RenderNodeCamera::build(*m_Scene, m_Info.camera));
    m_Graph.add("builtin/batch", m_Info.batch.getRenderNode());

    m_Graph.add(
        std::format("{}/buffer", Output),
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_Info.batch.getInstanceBuffer(),
                            {
                                .stage = Physbuzz::RenderNode::Stage::Transfer,
                            },
                        },
                    },
                    .output = {
                        {
                            PipelineCollisionHighlight::ResourceHighlightBuffer,
                            {
                                .stage = Physbuzz::RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .prepare = [this](const Physbuzz::RenderContext &context) {
                ZoneScopedN("CollisionHighlight/Execute");
                TracyVkZone(context.tracy, context.command, "CollisionHighlight");

                m_Highlighted.clear();
                m_Highlighted.reserve(m_Info.batch.getInstanceObjects().size());

                for (const auto &instance : m_Info.batch.getInstanceObjects()) {
                    m_Highlighted.emplace_back<PipelineCollisionHighlight::HighlightedObject>({
                        .highlighted = true, // TODO actually check collision
                    });
                }

                std::size_t highlightBufferSize = m_Highlighted.size() * sizeof(PipelineCollisionHighlight::HighlightedObject);
                PipelineCollisionHighlight::ResourceHighlightBuffer->rebuild(context, highlightBufferSize);
            },
            .execute = [this](const Physbuzz::RenderContext &context) {
                PipelineCollisionHighlight::ResourceHighlightBuffer->update(context, m_Highlighted);
            },
        });

    m_Graph.add(
        Output,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            Physbuzz::Builtin::RenderNodeCamera::ResourceBuffer,
                            {
                                .stage = Physbuzz::RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            PipelineCollisionHighlight::ResourceHighlightBuffer,
                            {
                                .stage = Physbuzz::RenderNode::Stage::Fragment,
                            },
                        },
                        {
                            m_Info.batch.getIndirectBuffer(),
                            {
                                .stage = Physbuzz::RenderNode::Stage::Indirect,
                            },
                        },
                        {
                            m_Info.batch.getInstanceBuffer(),
                            {
                                .stage = Physbuzz::RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .execute = [this](const Physbuzz::RenderContext &context) {
                ZoneScopedN("CollisionHighlight/Execute");
                TracyVkZone(context.tracy, context.command, "CollisionHighlight");

                std::lock_guard<std::mutex> lock(Physbuzz::ResourceRegistry<Physbuzz::GraphicsPipeline>::ReloadMutex);

                const Physbuzz::Image::Data &data = context.depth->getRingData()[context.frameInFlight].image.getData();

                std::array colorAttachments = {
                    vk::RenderingAttachmentInfo{
                        .imageView = context.color.view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eLoad,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
                    },
                };

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
                    .pDepthAttachment = {},
                    .pStencilAttachment = {},
                });

                // bind resources
                PipelineCollisionHighlight::Resource->bind(context);
                Physbuzz::App::LayoutAllocator.bind(context, PipelineCollisionHighlight::Resource);

                // draw
                m_Info.batch.draw(context);

                context.command.endRendering();
            },
        });

    success &= m_Graph.compile();

    if (success) {
        success &= Physbuzz::App::LayoutAllocator.write(
            PipelineCollisionHighlight::ResourceLayoutFrame,
            Physbuzz::Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= Physbuzz::App::LayoutAllocator.write(
            PipelineCollisionHighlight::ResourceLayoutFrame,
            PipelineCollisionHighlight::ResourceHighlightBuffer,
            1);

        success &= Physbuzz::App::LayoutAllocator.write(
            PipelineCollisionHighlight::ResourceLayoutFrame,
            m_Info.batch.getInstanceBuffer(),
            2);
    }

    return success;
}

const Physbuzz::RenderGraph &CollisionHighlight::getGraph() {
    return m_Graph;
}

bool CollisionHighlight::destroy() {
    m_Info.window->eraseCallback<Physbuzz::WindowSwapchainResizeEvent>(m_Events.resize);

    bool success = true;

    success &= Physbuzz::ResourceRegistry<Physbuzz::DynamicBuffer>::erase(PipelineCollisionHighlight::ResourceHighlightBuffer);

    return success;
}
