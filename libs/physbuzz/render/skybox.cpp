#include "skybox.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/descriptors/texture.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/pipeline.hpp"
#include "components/camera.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

SkyboxRenderer::SkyboxRenderer(Scene *scene, const Info &info)
    : System<>(scene),
      m_Info(info) {}

bool SkyboxRenderer::build() {
    ResourceContext *resourceContext = m_Scene->getSystem<ResourceContext>();

    if (!buildResources()) {
        Logger::ERROR("[SkyboxRenderer] Could not build skybox pipeline.");
        return false;
    }

    bool success = true;

    Resources::Specialization specialization = {
        .isCubemap = m_Info.skybox->getInfo().type == Texture::Type::Cube,
    };

    success &= m_Resources.pipeline->specialize(specialization);

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([this](const WindowSwapchainResizeEvent &event) {
            ResourceContext *resourceContext = m_Scene->getSystem<ResourceContext>();

            const auto [camera] = m_Scene->getComponent<CameraComponent>(resourceContext->getInfo().cameraId);
            camera.resize(event.resolution);
        }),
    };

    const ResourceContext::Resources &resources = resourceContext->getResouces();

    m_Graph.add("builtin/camera", resourceContext->buildCameraNode());

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
                    },
                },
            },
            .execute = [this](const RenderContext &context) {
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
                m_Resources.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.pipeline);

                // cubemap embedded within shader
                context.command.draw(36, 1, 0, 0);

                context.command.endRendering();
            },
        });

    success &= m_Graph.compile();

    if (success) {
        if (specialization.isCubemap) {
            success &= App::LayoutAllocator.write(
                m_Resources.layoutTexture,
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
                m_Resources.layoutTexture,
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
            m_Resources.layoutFrame,
            resources.camera,
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

const SkyboxRenderer::Resources &SkyboxRenderer::getResources() const {
    return m_Resources;
}

const SkyboxRenderer::Info &SkyboxRenderer::getInfo() const {
    return m_Info;
}

bool SkyboxRenderer::buildResources() {
    auto layoutFrame = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/skybox/frame",
        {{
            .bindings = {
                {
                    // camera
                    .type = DescriptorLayout::Type::eUniformBuffer,
                    .range = sizeof(ResourceContext::CameraBufferData),
                },
            },
        }});

    if (!layoutFrame) {
        return false;
    }

    auto layoutTexture = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/skybox/texture",
        {{
            .bindings = {
                {
                    // skybox (cubemap)
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .flags = DescriptorLayout::DescriptorBindingFlags::ePartiallyBound,
                },
                {
                    // skybox (equirectangular)
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .flags = DescriptorLayout::DescriptorBindingFlags::ePartiallyBound,
                },
            },
            .lifetime = DescriptorLayout::Lifetime::Global,
        }});

    if (!layoutTexture) {
        return false;
    }

    auto pipeline = ResourceRegistry<GraphicsPipeline>::insert(
        "builtin/skybox",
        {
            {
                .module = "builtin/render/skybox",
                .specialization = {
                    .offsets = {
                        offsetof(Resources::Specialization, isCubemap),
                    },
                    .size = sizeof(Resources::Specialization),
                },
            },
            {
                .layouts = {
                    .resources = {
                        *layoutTexture,
                        *layoutFrame,
                    },
                },
            },
        });

    if (!pipeline) {
        return false;
    }

    m_Resources = {
        .layoutTexture = std::move(*layoutTexture),
        .layoutFrame = std::move(*layoutFrame),
        .pipeline = std::move(*pipeline),
    };

    return true;
}

} // namespace Physbuzz
