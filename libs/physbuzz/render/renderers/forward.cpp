#include "forward.hpp"

#include "../../ecs/scene.hpp"
#include "../../events/window.hpp"
#include "../camera.hpp"
#include "../layout.hpp"
#include "../layouts/shaderbuffer.hpp"
#include "../model.hpp"
#include "../renderer.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineForward::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource.getIdentifier())) {
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
                        .type = Physbuzz::PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(CameraBuffer),
                    },
                    {
                        // lights
                        .type = Physbuzz::PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(LightBuffer),
                    },
                },
            }});
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutObject)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutObject,
            {{
                .bindings = {
                    {
                        // material
                        .type = Physbuzz::PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(MaterialBuffer),
                    },
                    {
                        // instance
                        .type = Physbuzz::PipelineLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    if (!ResourceRegistry<ShaderBuffer>::contains(ResourceBufferCamera)) {
        success &= ResourceRegistry<ShaderBuffer>::insert(
            ResourceBufferCamera,
            ShaderBuffer::Info<CameraBuffer>{
                .type = ShaderBuffer::Type::Constant,
            });
    }

    if (!ResourceRegistry<ShaderBuffer>::contains(ResourceBufferLight)) {
        success &= ResourceRegistry<ShaderBuffer>::insert(
            ResourceBufferLight,
            ShaderBuffer::Info<LightBuffer>{
                .type = ShaderBuffer::Type::Constant,
            });
    }

    if (!ResourceRegistry<ShaderBuffer>::contains(ResourceBufferMaterial)) {
        success &= ResourceRegistry<ShaderBuffer>::insert(
            ResourceBufferMaterial,
            ShaderBuffer::Info<MaterialBuffer>{
                .type = ShaderBuffer::Type::Constant,
            });
    }

    if (!ResourceRegistry<ShaderBuffer>::contains(ResourceBufferModel)) {
        success &= ResourceRegistry<ShaderBuffer>::insert(
            ResourceBufferModel,
            ShaderBuffer::Info<ModelBuffer>{
                .type = ShaderBuffer::Type::Structured,
            });
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .module = "builtin/forward",
            .description = &Model::Vertex::Description,
            .layouts = {
                .resources = {
                    ResourceLayoutFrame,
                    ResourceLayoutObject,
                },
                .pushConstantRanges = {{
                    .stageFlags = RenderPipeline::PushConstantsStageFlags::eAll,
                    .size = sizeof(PushConstantBuffer),
                }},
            },
        }});

    return success;
}

} // namespace Builtin

std::mutex PipelineReloadMutex;

ForwardRenderer::ForwardRenderer(const Info &info)
    : m_Info(info) {}

bool ForwardRenderer::build() {
    // build pipeline
    if (!Builtin::RenderPipelineForward::build()) {
        Logger::ERROR("[Renderer] Could not build forward shader pipeline.");
        return false;
    }

    bool success = true;

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutFrame,
        Builtin::RenderPipelineForward::ResourceBufferCamera,
        0);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutFrame,
        Builtin::RenderPipelineForward::ResourceBufferLight,
        1);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutObject,
        Builtin::RenderPipelineForward::ResourceBufferMaterial,
        0);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutObject,
        Builtin::RenderPipelineForward::ResourceBufferModel,
        1);

    if (success) {
        m_Events = {
            .resize = m_Scene->getSystem<Renderer>()->getInfo().window->addCallback<WindowSwapchainResizeEvent>([&](const WindowSwapchainResizeEvent &event) {
                const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
                camera.resize(event.resolution);
            }),
            .pipelineReload = ResourceRegistry<RenderPipeline>::Events.addCallback<OnResourceReload>([&](const OnResourceReload &event) {
                Resource<RenderPipeline> pipeline = {event.identifier};

                // not bound to this pipeline
                if (pipeline != m_Info.pipeline) {
                    return;
                }

                if (event.action != WatchAction::Modified || !pipeline->isDependantFile(event.filePath)) {
                    return;
                }

                Logger::INFO("[RenderPipeline] Reloading resource '{}'.", event.identifier, event.filePath.string());
                ResourceID identifier = m_Info.pipeline.getIdentifier();

                std::size_t bracketOpen = identifier.rfind(" (");
                std::size_t bracketClose = identifier.rfind(")");
                if (bracketOpen != std::string::npos && bracketClose != std::string::npos) {
                    std::string num = identifier.substr(bracketOpen + 2, bracketClose - (bracketOpen + 2));
                    if (!num.empty() && std::all_of(num.begin(), num.end(), [](unsigned char c) {
                            return std::isdigit(c);
                        })) {
                        identifier = std::format("{} ({})", identifier.substr(0, bracketOpen), std::stoi(num) + 1);
                    } else {
                        identifier = std::format("{} (1)", identifier);
                    }
                } else {
                    identifier = std::format("{} (1)", identifier);
                }

                if (ResourceRegistry<RenderPipeline>::insert(identifier, m_Info.pipeline->getInfo())) {
                    PipelineReloadMutex.lock();

                    // replace pipeline (we dont destroy the old pipeline until the program exits)
                    m_Info.pipeline = identifier;

                    PipelineReloadMutex.unlock();
                } else {
                    Logger::WARNING("[ForwardRenderer] Failed to hot-reload render pipeline '{}'.", m_Info.pipeline.getIdentifier());
                }
            }),
        };
    }

    return success;
}

bool ForwardRenderer::destroy() {
    m_Scene->getSystem<Renderer>()->getInfo().window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);
    m_Scene->getSystem<Renderer>()->getInfo().window->eraseCallback<OnResourceReload>(m_Events.pipelineReload);
    return true;
}

void ForwardRenderer::render(const RenderContext &context) {
    // setup attachments
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

    std::uint64_t modelBufferSize = Builtin::RenderPipelineForward::ResourceBufferModel->getBuffers()[context.frameInFlight].getData().bufferInfo.size;
    std::uint64_t requiredModelBufferSize = m_Objects.size() * sizeof(Builtin::RenderPipelineForward::ModelBuffer);

    if (requiredModelBufferSize > modelBufferSize) {
        Builtin::RenderPipelineForward::ResourceBufferModel->resize(context, m_Objects.size());

        // attach again since its a new buffer
        m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
            context,
            Builtin::RenderPipelineForward::ResourceLayoutObject,
            Builtin::RenderPipelineForward::ResourceBufferModel,
            1);
    }

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

    // upload lighting data
    const std::vector<DirectionalLightComponent> &directionals = m_Scene->getComponentArray<DirectionalLightComponent>();
    const std::vector<PointLightComponent> &points = m_Scene->getComponentArray<PointLightComponent>();
    const std::vector<SpotLightComponent> &spots = m_Scene->getComponentArray<SpotLightComponent>();

    Builtin::RenderPipelineForward::LightBuffer lightBuffer = {};
    std::copy_n(directionals.begin(), std::min(directionals.size(), lightBuffer.directionals.size()), lightBuffer.directionals.begin());
    std::copy_n(points.begin(), std::min(points.size(), lightBuffer.points.size()), lightBuffer.points.begin());
    std::copy_n(spots.begin(), std::min(spots.size(), lightBuffer.spots.size()), lightBuffer.spots.begin());

    Builtin::RenderPipelineForward::ResourceBufferLight->update<Builtin::RenderPipelineForward::LightBuffer>(
        context, m_Scene->getSystem<Transfer>(),
        {lightBuffer});

    // upload camera
    const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    Builtin::RenderPipelineForward::ResourceBufferCamera->update<Builtin::RenderPipelineForward::CameraBuffer>(
        context, m_Scene->getSystem<Transfer>(),
        {{
            .position = camera.getInfo().view.position,
            .view = camera.getView(),
            .projection = camera.getProjection(),
        }});

    // upload instance/object data
    std::unordered_map<Resource<Mesh>, std::vector<Builtin::RenderPipelineForward::ModelBuffer>> instances;

    for (const auto &object : m_Objects) {
        const auto [render] = m_Scene->getComponent<RenderComponent>(object);

        const Model::Info &model = render.model.getInfo();

        for (const auto &mesh : model.meshes) {
            instances[mesh.resource].emplace_back<Builtin::RenderPipelineForward::ModelBuffer>({
                .model = render.transform.matrix,
                .invModel = glm::inverse(render.transform.matrix),
            });
        }
    }

    PipelineReloadMutex.lock();

    m_Info.pipeline->bind(context);

    Builtin::RenderPipelineForward::PushConstantBuffer pushConstants = {
        .materialId = 0,
    };

    std::uint32_t object = 0;

    for (const auto &[mesh, meshes] : instances) {
        Builtin::RenderPipelineForward::ResourceBufferMaterial->update<Builtin::RenderPipelineForward::MaterialBuffer>(
            context, m_Scene->getSystem<Transfer>(),
            {{
                .diffuse = {1.0f, 0.0f, 0.0f},
                .specular = {1.0f, 1.0f, 1.0f},
                .specularity = 128.0f,
            }});

        Builtin::RenderPipelineForward::ResourceBufferModel->update<Builtin::RenderPipelineForward::ModelBuffer>(
            context, m_Scene->getSystem<Transfer>(),
            meshes, object);

        m_Info.pipeline->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span{&pushConstants, 1}), 0);
        m_Scene->getSystem<PipelineLayoutAllocator>()->bind(context, m_Info.pipeline);

        if (mesh->getDescription() != m_Info.pipeline->getInfo().description) {
            Logger::ERROR("[ForwardRenderer] Incompatible vertex state descriptions.");
            continue;
        }

        mesh->draw(context, meshes.size(), object);

        object += meshes.size() + object;
    }

    PipelineReloadMutex.unlock();

    context.command.endRendering();
}

} // namespace Physbuzz
