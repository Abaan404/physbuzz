#include "collision_highlight.hpp"

#include "../collision.hpp"
#include <physbuzz/app/application.hpp>
#include <physbuzz/events/window.hpp>
#include <physbuzz/graphics/descriptors/dynamic.hpp>
#include <physbuzz/graphics/layout.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/graphics/pipeline.hpp>
#include <physbuzz/render/components/camera.hpp>
#include <tracy/Tracy.hpp>

CollisionHighlight::CollisionHighlight(Physbuzz::Scene *scene, const Info &info)
    : Physbuzz::System<>(scene),
      m_Info(info) {}

bool CollisionHighlight::build() {
    Physbuzz::ResourceContext *resourceContext = m_Scene->getSystem<Physbuzz::ResourceContext>();

    if (!buildResources()) {
        Physbuzz::Logger::ERROR("[CollisionHighlight] Could not build collision highlight pipeline.");
        return false;
    }

    m_Events = {
        .resize = m_Info.window->addCallback<Physbuzz::WindowSwapchainResizeEvent>([this](const Physbuzz::WindowSwapchainResizeEvent &event) {
            Physbuzz::ResourceContext *resourceContext = m_Scene->getSystem<Physbuzz::ResourceContext>();

            const auto [camera] = m_Scene->getComponent<Physbuzz::CameraComponent>(resourceContext->getInfo().cameraId);
            camera.resize(event.resolution);
        }),
    };

    bool success = true;

    const Physbuzz::ResourceContext::Resources &resources = resourceContext->getResouces();

    m_Graph.add("builtin/camera", resourceContext->buildCameraNode());
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
                            m_Resources.highlightBuffer,
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

                std::vector<Physbuzz::Contact> contacts = m_Scene->getSystem<Collision>()->getContacts();

                for (const auto &instance : m_Info.batch.getInstanceObjects()) {
                    Physbuzz::ObjectID object = m_Info.objects[instance];

                    Resources::HighlightedObject &highlighted = m_Highlighted.emplace_back<Resources::HighlightedObject>({
                        .highlighted = false,
                    });

                    for (const auto &contact : contacts) {
                        if (contact.object1 == object || contact.object2 == object) {
                            highlighted.highlighted = true;
                            break;
                        }
                    }
                }

                std::size_t highlightBufferSize = m_Highlighted.size() * sizeof(Resources::HighlightedObject);
                m_Resources.highlightBuffer->rebuild(context, highlightBufferSize);
            },
            .execute = [this](const Physbuzz::RenderContext &context) {
                m_Resources.highlightBuffer->update(context, m_Highlighted);
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
                                .stage = Physbuzz::RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            m_Resources.highlightBuffer,
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
                m_Resources.pipeline->bind(context);
                Physbuzz::App::LayoutAllocator.bind(context, m_Resources.pipeline);

                // draw
                m_Info.batch.draw(context);

                context.command.endRendering();
            },
        });

    success &= m_Graph.compile();

    if (success) {
        success &= Physbuzz::App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            resources.camera,
            0);

        success &= Physbuzz::App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            m_Resources.highlightBuffer,
            1);

        success &= Physbuzz::App::LayoutAllocator.write(
            m_Resources.layoutFrame,
            m_Info.batch.getInstanceBuffer(),
            2);
    }

    return success;
}

const Physbuzz::RenderGraph &CollisionHighlight::getGraph() {
    return m_Graph;
}

const CollisionHighlight::Resources &CollisionHighlight::getResources() const {
    return m_Resources;
}

bool CollisionHighlight::destroy() {
    m_Info.window->eraseCallback<Physbuzz::WindowSwapchainResizeEvent>(m_Events.resize);
    return true;
}

bool CollisionHighlight::buildResources() {
    auto highlightBuffer = Physbuzz::ResourceRegistry<Physbuzz::DynamicBuffer>::insert(
        "collision_highlight/highlight",
        {{
            .type = Physbuzz::DynamicBuffer::Type::Structured,
        }},
        sizeof(Resources::HighlightedObject));

    if (!highlightBuffer) {
        return false;
    }

    auto layoutFrame = Physbuzz::ResourceRegistry<Physbuzz::DescriptorLayout>::insert(
        "collision_highlight/frame",
        {{
            .bindings = {
                {
                    // camera
                    .type = Physbuzz::DescriptorLayout::Type::eUniformBuffer,
                    .range = sizeof(Physbuzz::ResourceContext::CameraBufferData),
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

    if (!layoutFrame) {
        return false;
    }

    auto pipeline = Physbuzz::ResourceRegistry<Physbuzz::GraphicsPipeline>::insert(
        "collision_highlight",
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
                        *layoutFrame,
                    },
                },
            },
        });

    if (!pipeline) {
        return false;
    }

    m_Resources = {
        .layoutFrame = std::move(*layoutFrame),
        .highlightBuffer = std::move(*highlightBuffer),
        .pipeline = std::move(*pipeline),
    };

    return true;
}
