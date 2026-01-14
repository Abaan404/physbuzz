#include "forward.hpp"

#include "../../ecs/scene.hpp"
#include "../../events/window.hpp"
#include "../../window/window.hpp"
#include "../camera.hpp"
#include "../layout.hpp"
#include "../layouts/dynamic.hpp"
#include "../layouts/static.hpp"
#include "../model.hpp"
#include "../renderer.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineForward::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutGlobal)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutGlobal,
            {{
                .bindings = {
                    {
                        // textures
                        .type = PipelineLayout::Type::eCombinedImageSampler,
                        .flags = PipelineLayout::BindingFlagBits::ePartiallyBound | PipelineLayout::BindingFlagBits::eUpdateAfterBind,
                        .count = 32,
                    },
                },
                .flags = PipelineLayout::Flags::eUpdateAfterBindPool,
                .lifetime = PipelineLayout::Lifetime::Global,
            }});
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(CameraBuffer),
                    },
                    {
                        // lights
                        .type = PipelineLayout::Type::eUniformBuffer,
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
                        // instance
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    if (!ResourceRegistry<StaticBuffer>::contains(ResourceBufferMaterials)) {
        success &= ResourceRegistry<StaticBuffer>::insert(
            ResourceBufferMaterials,
            {},
            sizeof(MaterialBuffer));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferCamera)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferCamera,
            {{
                .type = DynamicBuffer::Type::Constant,
            }},
            sizeof(CameraBuffer));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferLight)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferLight,
            {{
                .type = DynamicBuffer::Type::Constant,
            }},
            sizeof(LightBuffer));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferModel)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferModel,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(ModelBuffer));
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/forward",
            .description = &Model::Vertex::Description,
            .layouts = {
                .resources = {
                    ResourceLayoutGlobal,
                    ResourceLayoutFrame,
                    ResourceLayoutObject,
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
        Builtin::RenderPipelineForward::ResourceBufferModel,
        0);

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
                    Logger::WARNING("[ForwardRenderer] Failed to hot-reload render pipeline '{}'.", m_Info.pipeline);
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

    // batch objects
    std::unordered_map<Resource<Mesh>, std::vector<Builtin::RenderPipelineForward::ModelBuffer>> instances;

    std::vector<Builtin::RenderPipelineForward::ModelBuffer> instanceBuffers;
    std::vector<std::pair<Resource<Mesh>, std::size_t>> instanceSizes;

    std::size_t meshCount = 0;
    for (const auto &object : m_Objects) {
        const auto [render] = m_Scene->getComponent<RenderComponent>(object);

        const Model::Info &model = render.model.getInfo();

        for (const auto &mesh : model.meshes) {
            std::unordered_map<TextureType, std::vector<std::uint32_t>> materialTextureIds;

            // fetch texture ids
            for (const auto &[type, textures] : mesh.material->textures) {
                materialTextureIds[type].reserve(textures.size());

                for (const auto &texture : textures) {
                    // new texture loaded into table, map to bindless descriptor
                    if (m_Textures.add(texture)) {
                        m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
                            Builtin::RenderPipelineForward::ResourceLayoutGlobal,
                            texture,
                            0, m_Textures.query(texture));
                    }

                    materialTextureIds.at(type).emplace_back(m_Textures.query(texture));
                }
            }

            // fetch material ids
            if (m_Materials.add(mesh.material)) {
                // create a new material buffer
                Builtin::RenderPipelineForward::MaterialBuffer material = {
                    .diffuseTextureIds = {},
                    .specularTextureIds = {},
                    .specularity = mesh.material->shininess,
                };

                for (const auto &[type, textureIds] : materialTextureIds) {
                    switch (type) {
                    case TextureType::Diffuse:
                        std::copy_n(textureIds.begin(), std::min(textureIds.size(), material.diffuseTextureIds.size()), material.diffuseTextureIds.begin());
                        break;

                    case TextureType::Specular:
                        std::copy_n(textureIds.begin(), std::min(textureIds.size(), material.specularTextureIds.size()), material.specularTextureIds.begin());
                        break;

                    default:
                        Logger::WARNING("[ForwardRenderer] Unhandled texture found {}", Model::getTextureTypeName(type));
                        break;
                    }
                }

                // resize if needed
                Builtin::RenderPipelineForward::ResourceBufferMaterials->resize(
                    context, m_Scene->getSystem<Transfer>(),
                    m_Materials.size() * sizeof(Builtin::RenderPipelineForward::MaterialBuffer));

                // upload material data
                Builtin::RenderPipelineForward::ResourceBufferMaterials->update<Builtin::RenderPipelineForward::MaterialBuffer>(
                    m_Scene->getSystem<Transfer>(),
                    {material}, m_Materials.query(mesh.material));
            }

            instances[mesh.mesh].emplace_back<Builtin::RenderPipelineForward::ModelBuffer>({
                .model = render.transform.matrix,
                .invModel = glm::inverse(render.transform.matrix),
                .materialOffset = m_Materials.query(mesh.material) * sizeof(Builtin::RenderPipelineForward::MaterialBuffer),
            });
        }

        meshCount += model.meshes.size();
    }

    for (const auto &[mesh, buffers] : instances) {
        instanceSizes.emplace_back<std::pair<Resource<Mesh>, std::size_t>>({mesh, buffers.size()});
        instanceBuffers.insert(instanceBuffers.end(), std::make_move_iterator(buffers.begin()), std::make_move_iterator(buffers.end()));
    }

    // resize if needed
    if (Builtin::RenderPipelineForward::ResourceBufferModel->resize(context, m_Scene->getSystem<Transfer>(), meshCount * sizeof(Builtin::RenderPipelineForward::ModelBuffer))) {
        m_Scene->getSystem<PipelineLayoutAllocator>()->reattach(
            context,
            Builtin::RenderPipelineForward::ResourceLayoutObject,
            Builtin::RenderPipelineForward::ResourceBufferModel,
            0);
    }

    // upload lighting data
    const std::vector<DirectionalLightComponent> &directionals = m_Scene->getComponentArray<DirectionalLightComponent>();
    const std::vector<PointLightComponent> &points = m_Scene->getComponentArray<PointLightComponent>();
    const std::vector<SpotLightComponent> &spots = m_Scene->getComponentArray<SpotLightComponent>();

    Builtin::RenderPipelineForward::LightBuffer lightBuffer = {};
    std::copy_n(directionals.begin(), std::min(directionals.size(), lightBuffer.directionals.size()), lightBuffer.directionals.begin());
    std::copy_n(points.begin(), std::min(points.size(), lightBuffer.points.size()), lightBuffer.points.begin());
    std::copy_n(spots.begin(), std::min(spots.size(), lightBuffer.spots.size()), lightBuffer.spots.begin());

    // upload lights
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
    Builtin::RenderPipelineForward::ResourceBufferModel->update<Builtin::RenderPipelineForward::ModelBuffer>(
        context, m_Scene->getSystem<Transfer>(),
        instanceBuffers);

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
    PipelineReloadMutex.lock();

    m_Info.pipeline->bind(context);

    // record constants
    Builtin::RenderPipelineForward::PushConstants pushConstants = {
        .material = Builtin::RenderPipelineForward::ResourceBufferMaterials->getAddress(),
    };

    m_Info.pipeline->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);
    m_Scene->getSystem<PipelineLayoutAllocator>()->bind(context, m_Info.pipeline);

    PipelineReloadMutex.unlock();

    std::uint32_t object = 0;
    for (const auto &[mesh, batch] : instanceSizes) {
        if (mesh->getDescription() != m_Info.pipeline->getInfo().description) {
            Logger::ERROR("[ForwardRenderer] Incompatible vertex state descriptions.");
            continue;
        }

        mesh->draw(context, batch, object);
        object += batch;
    }

    context.command.endRendering();
}

} // namespace Physbuzz
