#include "skybox.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/descriptors/texture.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/pipeline.hpp"
#include "components/camera.hpp"
#include "nodes/camera.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool PipelineSkybox::build() {
    if (ResourceRegistry<GraphicsPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

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
                },
            }});
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(ResourceLayoutTexture)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            ResourceLayoutTexture,
            {{
                .bindings = {
                    {
                        // skybox (cubemap)
                        .type = DescriptorLayout::Type::eCombinedImageSampler,
                    },
                    {
                        // skybox (equirectangular)
                        .type = DescriptorLayout::Type::eCombinedImageSampler,
                    },
                },
                .lifetime = DescriptorLayout::Lifetime::Global,
            }});
    }

    success &= ResourceRegistry<GraphicsPipeline>::insert(
        Resource,
        {
            {
                .module = "builtin/render/skybox",
                .specialization = {
                    .offsets = {
                        offsetof(Specialization, isCubemap),
                    },
                    .size = sizeof(Specialization),
                },
            },
            {
                .layouts = {
                    .resources = {
                        ResourceLayoutTexture,
                        ResourceLayoutFrame,
                    },
                },
            },
        });

    return success;
}

} // namespace Builtin

SkyboxRenderer::SkyboxRenderer(const Info &info)
    : m_Info(info) {}

bool SkyboxRenderer::build() {
    // build pipeline
    if (!Builtin::PipelineSkybox::build()) {
        Logger::ERROR("[SkyboxRenderer] Could not build skybox pipeline.");
        return false;
    }

    bool success = true;

    Builtin::PipelineSkybox::Specialization specialization = {
        .isCubemap = m_Info.skybox->getInfo().type == Texture::Type::Cube,
    };

    success &= Builtin::PipelineSkybox::Resource->specialize(specialization);

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([&](const WindowSwapchainResizeEvent &event) {
            const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
            camera.resize(event.resolution);
        }),
    };

    m_Graph.add("builtin/camera", Builtin::RenderNodeCamera::build(m_Info.camera));

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
                    },
                },
            },
            .execute = [this](Scene *, const RenderContext &context) {
                ZoneScopedN("SkyboxRenderer/Execute");
                TracyVkZone(context.tracy, context.command, "SkyboxRenderer");

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
                    .loadOp = vk::AttachmentLoadOp::eLoad,
                    .storeOp = vk::AttachmentStoreOp::eDontCare,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

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
                    .pDepthAttachment = &depthAttachment,
                    .pStencilAttachment = {},
                });

                // bind resources
                Builtin::PipelineSkybox::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineSkybox::Resource);

                // cubemap embedded within shader
                context.command.draw(36, 1, 0, 0);

                context.command.endRendering();
            },
        });

    success &= m_Graph.compile();

    if (success) {
        if (specialization.isCubemap) {
            success &= App::LayoutAllocator.write(
                Builtin::PipelineSkybox::ResourceLayoutTexture,
                m_Info.skybox,
                {
                    .type = Image::ViewType::eCube,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .levelCount = Image::RemainingMipLevels,
                        .layerCount = 6,
                    },
                },
                0);
        } else {
            success &= App::LayoutAllocator.write(
                Builtin::PipelineSkybox::ResourceLayoutTexture,
                m_Info.skybox,
                {
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .levelCount = Image::RemainingMipLevels,
                        .layerCount = 1,
                    },
                },
                1);
        }

        success &= App::LayoutAllocator.write(
            Builtin::PipelineSkybox::ResourceLayoutFrame,
            Builtin::RenderNodeCamera::ResourceBuffer,
            0);
    }

    return success;
}

bool SkyboxRenderer::destroy() {
    m_Info.window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);
    return true;
}

const RenderGraph &SkyboxRenderer::getGraph() const {
    return m_Graph;
}

const SkyboxRenderer::Info &SkyboxRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
