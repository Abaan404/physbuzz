#include "skybox.hpp"

#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/pipeline.hpp"
#include "camera.hpp"
#include "nodes/camera.hpp"

namespace Physbuzz {

namespace Builtin {

VertexDescription RenderPipelineSkybox::VertexSkybox::Description = {{
    .attributes = {
        {
            .format = VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(VertexSkybox::position) / sizeof(decltype(VertexSkybox::position)::value_type),
            .offset = offsetof(VertexSkybox, position),
        },
    },
    .size = sizeof(VertexSkybox),
}};

bool RenderPipelineSkybox::build(const std::shared_ptr<Transfer> transfer) {
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

    // TODO embed mesh in shader
    if (!ResourceRegistry<Mesh>::contains(ResourceMesh)) {
        Mesh::Info<VertexSkybox> mesh = {
            .vertices = {
                {{-0.5f, -0.5f, -0.5f}},
                {{-0.5f, -0.5f, 0.5f}},
                {{-0.5f, 0.5f, 0.5f}},
                {{-0.5f, 0.5f, -0.5f}},

                {{0.5f, -0.5f, 0.5f}},
                {{0.5f, -0.5f, -0.5f}},
                {{0.5f, 0.5f, -0.5f}},
                {{0.5f, 0.5f, 0.5f}},

                {{0.5f, -0.5f, -0.5f}},
                {{-0.5f, -0.5f, -0.5f}},
                {{-0.5f, 0.5f, -0.5f}},
                {{0.5f, 0.5f, -0.5f}},

                {{-0.5f, -0.5f, 0.5f}},
                {{0.5f, -0.5f, 0.5f}},
                {{0.5f, 0.5f, 0.5f}},
                {{-0.5f, 0.5f, 0.5f}},

                {{-0.5f, -0.5f, -0.5f}},
                {{0.5f, -0.5f, -0.5f}},
                {{0.5f, -0.5f, 0.5f}},
                {{-0.5f, -0.5f, 0.5f}},

                {{0.5f, 0.5f, -0.5f}},
                {{-0.5f, 0.5f, -0.5f}},
                {{-0.5f, 0.5f, 0.5f}},
                {{0.5f, 0.5f, 0.5f}},
            },
            .indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20},
        };

        success &= ResourceRegistry<Mesh>::insert(ResourceMesh, mesh, transfer);
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/skybox",
            .description = &VertexSkybox::Description,
            .rasterization = {
                .cullMode = RenderPipeline::CullModeFlags::eNone,
            },
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
        if (!Builtin::RenderPipelineSkybox::build(m_Scene->getSystem<Transfer>())) {
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

    m_Graph.add(Builtin::RenderNodeCamera::Id, Builtin::RenderNodeCamera::build(m_Info.camera));

    m_Graph.add(
        "builtin/skybox",
        {
            .description = {
                .buffers = {
                    .input = {
                        Builtin::RenderNodeCamera::ResourceBuffer,
                    },
                },
            },
            .execute = [&](Scene *, const RenderContext &context) {
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
                    .pDepthAttachment = {},
                    .pStencilAttachment = {},
                });

                // bind resources
                m_Info.pipeline->bind(context);
                context.systems.allocator->bind(context, m_Info.pipeline);

                Builtin::RenderPipelineSkybox::ResourceMesh->draw(context, 1, 0);

                context.command.endRendering();
            },
        });

    bool success = true;

    success &= m_Graph.compile();

    if (success) {
        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineSkybox::ResourceLayoutTexture,
            m_Info.skybox,
            0);

        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
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
