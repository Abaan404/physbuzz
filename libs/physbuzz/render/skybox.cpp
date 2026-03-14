#include "skybox.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/pipeline.hpp"
#include "components/camera.hpp"
#include "nodes/camera.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineSkybox::build() {
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
                        .range = sizeof(Builtin::RenderNodeCamera::CameraBuffer),
                    },
                },
            }});
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutTexture)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutTexture,
            {{
                .bindings = {
                    {
                        // skybox
                        .type = PipelineLayout::Type::eCombinedImageSampler,
                    },
                },
                .lifetime = PipelineLayout::Lifetime::Global,
            }});
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/render/skybox",
            .layouts = {
                .resources = {
                    ResourceLayoutTexture,
                    ResourceLayoutFrame,
                },
            },
        }});

    return success;
}

} // namespace Builtin

SkyboxRenderer::SkyboxRenderer(const Info &info)
    : m_Info(info) {}

bool SkyboxRenderer::build() {
    // build pipeline
    if (m_Info.pipeline == Builtin::RenderPipelineSkybox::Resource) {
        if (!Builtin::RenderPipelineSkybox::build()) {
            Logger::ERROR("[SkyboxRenderer] Could not build skybox shader pipeline.");
            return false;
        }
    }

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

                std::lock_guard<std::mutex> lock(RenderPipeline::ReloadMutex);

                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = context.depth->getRingData()[context.frameInFlight].view,
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
                m_Info.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Info.pipeline);

                // cubemap embedded within shader
                context.command.draw(36, 1, 0, 0);

                context.command.endRendering();
            },
        });

    bool success = true;

    success &= m_Graph.compile();

    if (success) {
        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineSkybox::ResourceLayoutTexture,
            m_Info.skybox,
            0);

        success &= App::LayoutAllocator.write(
            Builtin::RenderPipelineSkybox::ResourceLayoutFrame,
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
