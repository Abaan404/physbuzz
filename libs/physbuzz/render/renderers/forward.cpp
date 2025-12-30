#include "forward.hpp"

#include "../../ecs/scene.hpp"
#include "../../events/window.hpp"
#include "../camera.hpp"
#include "../layout.hpp"
#include "../layouts/static.hpp"
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
                        .stage = Physbuzz::PipelineLayout::ShaderStageFlags::eAll,
                    },
                    {
                        // lights
                        .type = Physbuzz::PipelineLayout::Type::eStorageBuffer,
                        .stage = Physbuzz::PipelineLayout::ShaderStageFlags::eAll,
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
                        .stage = Physbuzz::PipelineLayout::ShaderStageFlags::eAll,
                    },
                    {
                        // instance
                        .type = Physbuzz::PipelineLayout::Type::eStorageBuffer,
                        .stage = Physbuzz::PipelineLayout::ShaderStageFlags::eAll,
                    },
                },
            }});
    }

    if (!ResourceRegistry<StaticBuffer>::contains(ResourceBufferCamera)) {
        success &= ResourceRegistry<StaticBuffer>::insert(
            ResourceBufferCamera,
            StaticBuffer::Info<CameraBuffer>{
                .count = 1,
                .type = StaticBuffer::Type::Constant,
            });
    }

    if (!ResourceRegistry<StaticBuffer>::contains(ResourceBufferLight)) {
        success &= ResourceRegistry<StaticBuffer>::insert(
            ResourceBufferLight,
            StaticBuffer::Info<LightBuffer>{
                .count = 1,
                .type = StaticBuffer::Type::Structured,
            });
    }

    if (!ResourceRegistry<StaticBuffer>::contains(ResourceBufferMaterial)) {
        success &= ResourceRegistry<StaticBuffer>::insert(
            ResourceBufferMaterial,
            StaticBuffer::Info<MaterialBuffer>{
                .count = 1,
                .type = StaticBuffer::Type::Constant,
            });
    }

    if (!ResourceRegistry<StaticBuffer>::contains(ResourceBufferModel)) {
        success &= ResourceRegistry<StaticBuffer>::insert(
            ResourceBufferModel,
            StaticBuffer::Info<ModelBuffer>{
                .count = 1,
                .type = StaticBuffer::Type::Structured,
            });
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource.getIdentifier(),
        {{
            .module = "builtin/forward",
            .description = &Model::Vertex::Description,
            .layouts = {
                ResourceLayoutFrame,
                ResourceLayoutObject,
            },
        }});

    return success;
}

} // namespace Builtin

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
            .resize = m_Scene->getSystem<Renderer>()->getInfo().window->addCallback<WindowSwapchainResizeEvent>([&](const auto &event) {
                const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
                camera.resize(event.resolution);
            }),
        };
    }

    return success;
}

bool ForwardRenderer::destroy() {
    m_Scene->getSystem<Renderer>()->getInfo().window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);
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
        m_Scene->getSystem<Renderer>(), m_Scene->getSystem<Transfer>(),
        {lightBuffer});

    // upload camera
    const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    Builtin::RenderPipelineForward::ResourceBufferCamera->update<Builtin::RenderPipelineForward::CameraBuffer>(
        m_Scene->getSystem<Renderer>(), m_Scene->getSystem<Transfer>(),
        {{
            .position = camera.getInfo().view.position,
            .view = camera.getView(),
            .projection = camera.getProjection(),
        }});

    // upload instance/object data
    std::unordered_map<Resource<Model>, std::vector<Builtin::RenderPipelineForward::ModelBuffer>> instances;

    for (const auto &object : m_Objects) {
        const auto [render] = m_Scene->getComponent<RenderComponent>(object);

        instances[render.model].emplace_back<Builtin::RenderPipelineForward::ModelBuffer>({
            .model = render.transform.matrix,
            .invModel = glm::inverse(render.transform.matrix),
        });
    }

    for (const auto &[model, buffer] : instances) {
        Builtin::RenderPipelineForward::ResourceBufferMaterial->update<Builtin::RenderPipelineForward::MaterialBuffer>(
            m_Scene->getSystem<Renderer>(), m_Scene->getSystem<Transfer>(),
            {{
                .diffuse = {1.0f, 0.0f, 0.0f},
                .specular = {0.0f, 0.0f, 0.0f},
                .specularity = 0.0f,
            }});

        Builtin::RenderPipelineForward::ResourceBufferModel->update<Builtin::RenderPipelineForward::ModelBuffer>(
            m_Scene->getSystem<Renderer>(), m_Scene->getSystem<Transfer>(),
            buffer);

        Builtin::RenderPipelineForward::Resource->bind(context.command);
        m_Scene->getSystem<PipelineLayoutAllocator>()->bind(context.command, Builtin::RenderPipelineForward::Resource, m_Scene->getSystem<Renderer>());

        for (const auto &[mesh, _] : model->getMeshs()) {
            if (mesh.getDescription() != Builtin::RenderPipelineForward::Resource->getInfo().description) {
                Logger::ERROR("[ForwardRenderer] Incompatible vertex state descriptions.");
                return;
            }

            model->draw(context.command, buffer.size());
        }
    }

    context.command.endRendering();
}

} // namespace Physbuzz
