#include "deferred.hpp"

#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/material.hpp"
#include "camera.hpp"
#include "lighting.hpp"
#include "nodes/camera.hpp"
#include "nodes/lights.hpp"
#include "nodes/model.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineDeferred::Geometry::build() {
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
                },
            }});
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutGBuffers)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutGBuffers,
            {{
                .bindings = {
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
                .lifetime = PipelineLayout::Lifetime::Global,
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
                    ResourceLayoutGBuffers,
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

DeferredRenderer::DeferredRenderer(const Info &info)
    : m_Info(info) {}

bool DeferredRenderer::build() {
    // build pipeline
    if (m_Info.geometry == Builtin::RenderPipelineDeferred::Geometry::Resource) {
        if (!Builtin::RenderPipelineDeferred::Geometry::build()) {
            Logger::ERROR("[DeferredRenderer] Could not build the geometry pipeline.");
            return false;
        }
    }

    if (m_Info.lighting == Builtin::RenderPipelineDeferred::Lighting::Resource) {
        if (!Builtin::RenderPipelineDeferred::Lighting::build()) {
            Logger::ERROR("[DeferredRenderer] Could not build the lighting pipeline.");
            return false;
        }
    }

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([&](const WindowSwapchainResizeEvent &event) {
            const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
            camera.resize(event.resolution);
        }),
    };

    m_Graph.add("builtin/deferred/camera", Builtin::RenderNodeCamera::build(m_Info.camera));
    m_Graph.add("builtin/deferred/lights", Builtin::RenderNodeLights::build());
    m_Graph.add("builtin/deferred/models", Builtin::RenderNodeModels::build(m_Objects, m_Batches));

    m_Graph.add(
        "builtin/deferred/gbuffers",
        {
            .description = {
                .textures = {
                    .output = {
                        {
                            "builtin/deferred/gBuffer0",
                            {
                                {
                                    .type = Texture::Type::Attachment,
                                    .sampler = Texture::Sampler::None,
                                    .format = Texture::Format::eR16G16B16A16Sfloat,
                                },
                                glm::uvec3{1, 1, 1},
                            },
                        },
                        {
                            "builtin/deferred/gBuffer1",
                            {
                                {
                                    .type = Texture::Type::Attachment,
                                    .sampler = Texture::Sampler::None,
                                    .format = Texture::Format::eR8G8B8A8Snorm,
                                },
                                glm::uvec3{1, 1, 1},
                            },
                        },
                        {
                            "builtin/deferred/gBuffer2",
                            {
                                {
                                    .type = Texture::Type::Attachment,
                                    .sampler = Texture::Sampler::None,
                                    .format = Texture::Format::eR8G8B8A8Unorm,
                                },
                                glm::uvec3{1, 1, 1},
                            },
                        },
                    },
                },
                .buffers = {
                    .input = {
                        Builtin::RenderNodeCamera::ResourceBuffer,
                        Builtin::RenderNodeModels::ResourceBuffer,
                    },
                },
            },
            .execute = [&](Scene *scene, const RenderContext &context) {
                std::array gBuffers = {
                    Resource<Texture>("builtin/deferred/gBuffer0"),
                    Resource<Texture>("builtin/deferred/gBuffer1"),
                    Resource<Texture>("builtin/deferred/gBuffer2"),
                };

                std::array<vk::RenderingAttachmentInfo, gBuffers.size()> colorAttachments;
                std::array<vk::ImageMemoryBarrier2, gBuffers.size()> layoutBarriers;

                for (std::size_t i = 0; i < gBuffers.size(); i++) {
                    glm::uvec3 resolution = gBuffers[i]->getSize();
                    if (resolution.x != context.extent.width || resolution.y != context.extent.height) {
                        gBuffers[i]->resize(context, {context.extent.width, context.extent.height, 1});
                    }

                    colorAttachments[i] = {
                        .imageView = gBuffers[i]->getData().view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eClear,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
                    };

                    layoutBarriers[i] = {
                        .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                        .srcAccessMask = vk::AccessFlagBits2::eNone,
                        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                        .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                        .oldLayout = vk::ImageLayout::eAttachmentOptimal,
                        .newLayout = vk::ImageLayout::eAttachmentOptimal,
                        .image = gBuffers[i]->getImage().getData().image,
                        .subresourceRange = {
                            .aspectMask = vk::ImageAspectFlagBits::eColor,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                    };
                }

                context.command.pipelineBarrier2({
                    .imageMemoryBarrierCount = layoutBarriers.size(),
                    .pImageMemoryBarriers = layoutBarriers.data(),
                });

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = context.depth.view,
                    .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eDontCare,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

                // record constants
                Builtin::RenderPipelineDeferred::Geometry::PushConstants pushConstants = {
                    .materialBaseAddress = context.materialAllocator->getMaterialBuffer().getAddress(),
                };

                m_Info.geometry->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

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
                m_Info.geometry->bind(context);
                context.systems.allocator->bind(context, m_Info.geometry);

                std::uint32_t object = 0;
                for (const auto &[mesh, batch] : m_Batches) {
                    if (mesh->getDescription() != m_Info.geometry->getInfo().description) {
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
        "builtin/deferred/lighting",
        {
            .description = {
                .textures = {
                    .input = {
                        "builtin/deferred/gBuffer0",
                        "builtin/deferred/gBuffer1",
                        "builtin/deferred/gBuffer2",
                    },
                },
                .buffers = {
                    .input = {
                        Builtin::RenderNodeCamera::ResourceBuffer,
                        Builtin::RenderNodeLights::ResourceBufferDirectional,
                        Builtin::RenderNodeLights::ResourceBufferPoint,
                        Builtin::RenderNodeLights::ResourceBufferSpot,
                    },
                },
            },
            .execute = [&](Scene *scene, const RenderContext &context) {
                const std::vector<DirectionalLightComponent> &directionals = scene->getComponentArray<DirectionalLightComponent>();
                const std::vector<PointLightComponent> &points = scene->getComponentArray<PointLightComponent>();
                const std::vector<SpotLightComponent> &spots = scene->getComponentArray<SpotLightComponent>();

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
                    .pDepthAttachment = &depthAttachment,
                    .pStencilAttachment = {},
                });

                // bind resources
                m_Info.lighting->bind(context);
                context.systems.allocator->bind(context, m_Info.lighting);

                // record constants
                Builtin::RenderPipelineDeferred::Lighting::PushConstants pushConstants = {
                    .directionalCount = static_cast<std::uint32_t>(directionals.size()),
                    .spotCount = static_cast<std::uint32_t>(spots.size()),
                    .pointCount = static_cast<std::uint32_t>(points.size()),
                };

                m_Info.lighting->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

                // draw one triangle
                context.command.draw(3, 1, 0, 0);

                context.command.endRendering();
            },
        });

    bool success = true;

    success &= m_Graph.compile("builtin/deferred/lighting");

    if (success) {
        // geometry
        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Geometry::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Geometry::ResourceLayoutFrame,
            Builtin::RenderNodeModels::ResourceBuffer,
            1);

        // lighting
        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutGBuffers,
            Resource<Texture>("builtin/deferred/gBuffer0"),
            0);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutGBuffers,
            Resource<Texture>("builtin/deferred/gBuffer1"),
            1);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutGBuffers,
            Resource<Texture>("builtin/deferred/gBuffer2"),
            2);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            1);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferPoint,
            2);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferSpot,
            3);
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
