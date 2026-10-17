#include "shadow.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../graphics/layout.hpp"
#include "components/lights.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

VertexDescription ShadowRenderer::Vertex::Description = {{
    .attributes = {
        {
            .format = VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(Vertex::position) / sizeof(decltype(Vertex::position)::value_type),
            .offset = offsetof(Vertex, position),
        },
    },
    .size = sizeof(Vertex),
}};

ShadowRenderer::ShadowRenderer(Scene *scene, const Info &info)
    : System<ShadowComponent, TransformComponent>(scene),
      m_Info(info) {}

bool ShadowRenderer::build() {
    ResourceContext *resourceContext = m_Scene->getSystem<ResourceContext>();

    if (!buildResources()) {
        return false;
    }

    bool success = true;

    success &= m_Batch.build(m_Renderables);
    success &= m_Culling.build(*m_Scene);

    const auto updateCullingObjects = [this](...) {
        m_Culling.setObjects({
            .directionalLights = m_Scene->getObjects<DirectionalLightComponent>(),
            .pointLights = m_Scene->getObjects<PointLightComponent>(),
            .spotLights = m_Scene->getObjects<SpotLightComponent>(),
        });
    };

    m_Events = {
        m_Scene->addCallback<OnComponentSetEvent<DirectionalLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentSetEvent<PointLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentSetEvent<SpotLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentEraseEvent<DirectionalLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentEraseEvent<PointLightComponent>>(updateCullingObjects),
        m_Scene->addCallback<OnComponentEraseEvent<SpotLightComponent>>(updateCullingObjects),
    };

    m_Graph.add("builtin/lights", resourceContext->buildLightsNode());
    m_Graph.add("builtin/shadow/batch", m_Batch.getRenderNode());
    m_Graph.merge(m_Culling.getGraph());

    const ResourceContext::Resources &resources = resourceContext->getResouces();

    m_Graph.add(
        OutputDirectional,
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
                            resources.directionalLights,
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
                    .output = {
                        {
                            m_Resources.directional.attachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {{
                                    .aspectMask = Image::AspectFlags::eDepth,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                }},
                            },
                        },
                    },
                },
            },
            .execute = [this](const RenderContext &context) {
                ZoneScopedN("ShadowRenderer/Directional/Execute");
                TracyVkZone(context.tracy, context.command, "ShadowRenderer/Directional");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                const Image::Data &data = m_Resources.directional.attachment->getRingData()[context.frameInFlight].image.getData();

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = data.views.at({
                        .type = Image::ViewType::e2D,
                        .subresourceRange = {
                            .aspectMask = Image::AspectFlags::eDepth,
                            .levelCount = 1,
                            .layerCount = 1,
                        },
                    }),
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

                // TODO multiple objects
                ObjectID object = *m_Scene->getObjects<DirectionalLightComponent>().begin();

                // bind resources
                std::uint32_t frustumId = m_Culling.getFrustumId(object);

                Resources::Directional::PushConstants pushConstants = {
                    .objectCount = static_cast<std::uint32_t>(m_Batch.getObjectCount(context.frameInFlight)),
                    .cameraId = frustumId,
                };

                m_Resources.directional.pipeline->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eVertex, std::as_bytes(std::span(&pushConstants, 1)), 0);
                m_Resources.directional.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.directional.pipeline);

                // draw
                m_Batch.draw(context, m_Culling.getIndirectBuffer(), m_Culling.getIndirectOffset(frustumId, context.frameInFlight));

                context.command.endRendering();
            },
        });

    m_Graph.add(
        OutputPoint,
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
                            resources.pointLights,
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
                    .output = {
                        {
                            m_Resources.point.attachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                                .subresourceRanges = {
                                    {.aspectMask = Image::AspectFlags::eDepth, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
                                    {.aspectMask = Image::AspectFlags::eDepth, .levelCount = 1, .baseArrayLayer = 1, .layerCount = 1},
                                    {.aspectMask = Image::AspectFlags::eDepth, .levelCount = 1, .baseArrayLayer = 2, .layerCount = 1},
                                    {.aspectMask = Image::AspectFlags::eDepth, .levelCount = 1, .baseArrayLayer = 3, .layerCount = 1},
                                    {.aspectMask = Image::AspectFlags::eDepth, .levelCount = 1, .baseArrayLayer = 4, .layerCount = 1},
                                    {.aspectMask = Image::AspectFlags::eDepth, .levelCount = 1, .baseArrayLayer = 5, .layerCount = 1},
                                },
                            },
                        },
                    },
                },
            },
            .execute = [this](const RenderContext &context) {
                ZoneScopedN("ShadowRenderer/Point/Execute");
                TracyVkZone(context.tracy, context.command, "ShadowRenderer/Point");

                std::lock_guard<std::mutex> lock(ResourceRegistry<GraphicsPipeline>::ReloadMutex);

                context.command.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_Info.resolution.x), static_cast<float>(m_Info.resolution.y), 0.0f, 1.0f});
                context.command.setScissor(0, vk::Rect2D{{0, 0}, {m_Info.resolution.x, m_Info.resolution.y}});

                // TODO multiple objects
                ObjectID object = *m_Scene->getObjects<PointLightComponent>().begin();

                std::uint32_t frustumId = m_Culling.getFrustumId(object);

                const Image::Data &data = m_Resources.point.attachment->getRingData()[context.frameInFlight].image.getData();

                for (std::uint32_t faceId = 0; faceId < 6; faceId++) {
                    vk::RenderingAttachmentInfo depthAttachment = {
                        .imageView = data.views.at({
                            .type = Image::ViewType::e2D,
                            .subresourceRange = {
                                .aspectMask = Image::AspectFlags::eDepth,
                                .levelCount = 1,
                                .baseArrayLayer = faceId,
                                .layerCount = 1,
                            },
                        }),
                        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eClear,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                        .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                    };

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
                    m_Resources.point.pipeline->bind(context);
                    App::LayoutAllocator.bind(context, m_Resources.point.pipeline);

                    Resources::Point::PushConstants pushConstants = {
                        .objectCount = static_cast<std::uint32_t>(m_Batch.getObjectCount(context.frameInFlight)),
                        .cameraId = frustumId + faceId,
                        .faceId = faceId,
                    };

                    m_Resources.point.pipeline->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eVertex, std::as_bytes(std::span(&pushConstants, 1)), 0);

                    // draw
                    m_Batch.draw(context, m_Culling.getIndirectBuffer(), m_Culling.getIndirectOffset(frustumId + faceId, context.frameInFlight));

                    context.command.endRendering();
                }
            },
        });

    if (success) {
        success &= App::LayoutAllocator.write(
            m_Resources.directional.layout,
            resources.directionalLights,
            0);

        success &= App::LayoutAllocator.write(
            m_Resources.directional.layout,
            m_Batch.getInstanceBuffer(),
            1);

        success &= App::LayoutAllocator.write(
            m_Resources.directional.layout,
            m_Culling.getVisibleInstanceBuffer(),
            2);

        success &= App::LayoutAllocator.write(
            m_Resources.point.layout,
            resources.pointLights,
            0);

        success &= App::LayoutAllocator.write(
            m_Resources.point.layout,
            m_Batch.getInstanceBuffer(),
            1);

        success &= App::LayoutAllocator.write(
            m_Resources.point.layout,
            m_Culling.getVisibleInstanceBuffer(),
            2);
    }

    return success;
}

void ShadowRenderer::tick() {
    m_Renderables.clear();

    for (const auto &object : m_Objects) {
        const auto [shadow, transform] = m_Scene->getComponent<ShadowComponent, TransformComponent>(object);

        m_Renderables.emplace_back<Renderable>({
            .model = &shadow.model,
            .transform = &transform.transform,
        });
    }
}

bool ShadowRenderer::destroy() {
    bool success = true;

    success &= m_Batch.destroy();
    success &= m_Culling.destroy();

    return success;
}

const RenderBatch &ShadowRenderer::getBatch() const {
    return m_Batch;
}

const RenderGraph &ShadowRenderer::getGraph() const {
    return m_Graph;
}

const ShadowRenderer::Resources &ShadowRenderer::getResources() const {
    return m_Resources;
}

const ShadowRenderer::Info &ShadowRenderer::getInfo() const {
    return m_Info;
}

bool ShadowRenderer::buildResources() {
    auto directional = buildDirectionalResources(m_Info.resolution);

    if (!directional) {
        return false;
    }

    auto point = buildPointResources(m_Info.resolution);

    if (!point) {
        return false;
    }

    m_Resources = {
        .directional = std::move(*directional),
        .point = std::move(*point),
    };

    return true;
}

std::optional<ShadowRenderer::Resources::Directional> ShadowRenderer::buildDirectionalResources(const glm::uvec2 &resolution) {
    auto attachment = ResourceRegistry<Attachment>::insert(
        "builtin/shadow/directional",
        {{
            .usage = Attachment::Usage::Depth,
            .format = Attachment::Format::eD32Sfloat,
            .sampler = {{
                .type = Sampler::Type::Linear,
            }},
            .views = {
                {
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eDepth,
                        .levelCount = 1,
                        .layerCount = 1,
                    },
                },
            },
        }},
        resolution);

    if (!attachment) {
        return std::nullopt;
    }

    auto layout = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/shadow/directional/frame",
        {{
            .bindings = {
                {
                    // lights
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
            },
        }});

    if (!layout) {
        return std::nullopt;
    }

    auto pipeline = ResourceRegistry<GraphicsPipeline>::insert(
        "builtin/shadow/directional",
        {
            {
                .module = "builtin/render/shadow/directional",
            },
            {
                .description = &Vertex::Description,
                .rasterization = {
                    .depthBiasEnable = true,
                    .depthBiasConstantFactor = 6.0f,
                    .depthBiasClamp = 0.0f,
                    .depthBiasSlopeFactor = 1.5f,
                    .cullMode = GraphicsPipeline::CullModeFlags::eFront,
                },
                .blend = {
                    .attachments = {{}},
                },
                .formats = {
                    .color = {},
                    .depth = GraphicsPipeline::Format::eD32Sfloat,
                },
                .layouts = {
                    .resources = {
                        *layout,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eVertex,
                            .size = sizeof(Resources::Directional::PushConstants),
                        },
                    },
                },
                .inputs = {
                    .colors = {},
                },
            },
        });

    if (!pipeline) {
        return std::nullopt;
    }

    return std::make_optional<Resources::Directional>({
        .attachment = std::move(*attachment),
        .layout = std::move(*layout),
        .pipeline = std::move(*pipeline),
    });
}

std::optional<ShadowRenderer::Resources::Point> ShadowRenderer::buildPointResources(const glm::uvec2 &resolution) {
    std::vector<Image::ViewInfo> views;
    views.reserve(7);

    // create a view for each face to be rendered seperately
    for (std::uint32_t i = 0; i < 6; i++) {
        views.emplace_back<Image::ViewInfo>({
            .type = Image::ViewType::e2D,
            .subresourceRange = {
                .aspectMask = Image::AspectFlags::eDepth,
                .levelCount = 1,
                .baseArrayLayer = i,
                .layerCount = 1,
            },
        });
    }

    // the readable cube view
    views.emplace_back<Image::ViewInfo>({
        .type = Image::ViewType::eCube,
        .subresourceRange = {
            .aspectMask = Image::AspectFlags::eDepth,
            .levelCount = 1,
            .layerCount = 6,
        },
    });

    auto attachment = ResourceRegistry<Attachment>::insert(
        "builtin/shadow/point",
        {{
            .type = Attachment::Type::Cube,
            .usage = Attachment::Usage::Depth,
            .format = Attachment::Format::eD32Sfloat,
            .sampler = {{
                .type = Sampler::Type::Linear,
            }},
            .views = views,
        }},
        resolution);

    if (!attachment) {
        return std::nullopt;
    }

    auto layout = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/shadow/point/frame",
        {{
            .bindings = {
                {
                    // lights
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
            },
        }});

    if (!layout) {
        return std::nullopt;
    }

    auto pipeline = ResourceRegistry<GraphicsPipeline>::insert(
        "builtin/shadow/point",
        {
            {
                .module = "builtin/render/shadow/point",
            },
            {
                .description = &Vertex::Description,
                .rasterization = {
                    .depthBiasEnable = true,
                    .depthBiasConstantFactor = 6.0f,
                    .depthBiasClamp = 0.0f,
                    .depthBiasSlopeFactor = 1.5f,
                    .cullMode = GraphicsPipeline::CullModeFlags::eFront,
                },
                .blend = {
                    .attachments = {{}},
                },
                .formats = {
                    .color = {},
                    .depth = GraphicsPipeline::Format::eD32Sfloat,
                },
                .layouts = {
                    .resources = {
                        *layout,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eVertex,
                            .size = sizeof(Resources::Point::PushConstants),
                        },
                    },
                },
                .inputs = {
                    .colors = {},
                },
            },
        });

    if (!pipeline) {
        return std::nullopt;
    }

    return std::make_optional<Resources::Point>({
        .attachment = std::move(*attachment),
        .layout = std::move(*layout),
        .pipeline = std::move(*pipeline),
    });
}

} // namespace Physbuzz
