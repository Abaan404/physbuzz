#include "forward.hpp"

#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/descriptors/dynamic.hpp"
#include "../graphics/descriptors/static.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/renderer.hpp"
#include "camera.hpp"
#include "lighting.hpp"
#include "model.hpp"
#include "resources/common.hpp"

namespace Physbuzz {

ForwardRenderer::ForwardRenderer(const Info &info)
    : m_Info(info) {}

bool ForwardRenderer::build() {
    // build pipeline
    if (m_Info.pipeline == Builtin::RenderPipelineForward::Resource) {
        if (!Builtin::RenderPipelineForward::build()) {
            Logger::ERROR("[Renderer] Could not build forward shader pipeline.");
            return false;
        }
    }

    bool success = true;

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutFrame,
        Builtin::RenderPipelineForward::ResourceBufferCamera,
        0);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutFrame,
        Builtin::RenderPipelineForward::ResourceBufferDirectionalLights,
        1);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutFrame,
        Builtin::RenderPipelineForward::ResourceBufferPointLights,
        2);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineForward::ResourceLayoutFrame,
        Builtin::RenderPipelineForward::ResourceBufferSpotLights,
        3);

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
        };
    }

    return success;
}

bool ForwardRenderer::destroy() {
    m_Scene->getSystem<Renderer>()->getInfo().window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);
    return true;
}

void ForwardRenderer::render(const RenderContext &context) {
    // batch objects
    std::unordered_map<Resource<Mesh>, std::vector<Builtin::RenderPipelineForward::ModelBuffer>> instances;

    std::vector<Builtin::RenderPipelineForward::ModelBuffer> instanceBuffers;
    std::vector<std::pair<Resource<Mesh>, std::size_t>> instanceSizes;

    std::size_t meshCount = 0;
    for (const auto &object : m_Objects) {
        const auto [render] = m_Scene->getComponent<RenderComponent>(object);

        Builtin::RenderLayoutGlobal::refresh(context, render.model);

        const Model::Info &model = render.model.getInfo();
        for (const auto &mesh : model.meshes) {
            instances[mesh.mesh].emplace_back<Builtin::RenderPipelineForward::ModelBuffer>({
                .model = render.transform.matrix,
                .invModel = glm::inverse(render.transform.matrix),
                .materialIdx = Builtin::RenderLayoutGlobal::TableMaterial.query(mesh.material),
            });
        }

        meshCount += render.model.getInfo().meshes.size();
    }

    for (const auto &[mesh, buffers] : instances) {
        instanceSizes.emplace_back<std::pair<Resource<Mesh>, std::size_t>>({mesh, buffers.size()});
        instanceBuffers.insert(instanceBuffers.end(), std::make_move_iterator(buffers.begin()), std::make_move_iterator(buffers.end()));
    }

    std::size_t requiredModelSize = meshCount * sizeof(Builtin::RenderPipelineForward::ModelBuffer);
    if (Builtin::RenderPipelineForward::ResourceBufferModel->getSize(context) < requiredModelSize) {
        Builtin::RenderPipelineForward::ResourceBufferModel->resize(context, requiredModelSize);

        // retach for a new buffer
        context.systems.allocator->reattach(
            context,
            Builtin::RenderPipelineForward::ResourceLayoutObject,
            Builtin::RenderPipelineForward::ResourceBufferModel,
            0);
    }

    // upload lighting data
    const std::vector<DirectionalLightComponent> &directionals = m_Scene->getComponentArray<DirectionalLightComponent>();
    const std::vector<PointLightComponent> &points = m_Scene->getComponentArray<PointLightComponent>();
    const std::vector<SpotLightComponent> &spots = m_Scene->getComponentArray<SpotLightComponent>();

    std::size_t requiredDirectionalSize = directionals.size() * sizeof(DirectionalLightComponent);
    if (Builtin::RenderPipelineForward::ResourceBufferDirectionalLights->getSize(context) < requiredDirectionalSize) {
        Builtin::RenderPipelineForward::ResourceBufferDirectionalLights->resize(context, requiredDirectionalSize);

        // retach for a new buffer
        context.systems.allocator->reattach(
            context,
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderPipelineForward::ResourceBufferDirectionalLights,
            1);
    }

    std::size_t requiredPointsSize = points.size() * sizeof(PointLightComponent);
    if (Builtin::RenderPipelineForward::ResourceBufferPointLights->getSize(context) < requiredPointsSize) {
        Builtin::RenderPipelineForward::ResourceBufferPointLights->resize(context, requiredPointsSize);

        // retach for a new buffer
        context.systems.allocator->reattach(
            context,
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderPipelineForward::ResourceBufferPointLights,
            2);
    }

    std::size_t requiredSpotSize = spots.size() * sizeof(SpotLightComponent);
    if (Builtin::RenderPipelineForward::ResourceBufferSpotLights->getSize(context) < requiredSpotSize) {
        Builtin::RenderPipelineForward::ResourceBufferSpotLights->resize(context, requiredSpotSize);

        // retach for a new buffer
        context.systems.allocator->reattach(
            context,
            Builtin::RenderPipelineForward::ResourceLayoutFrame,
            Builtin::RenderPipelineForward::ResourceBufferSpotLights,
            3);
    }

    Builtin::RenderPipelineForward::ResourceBufferDirectionalLights->update<DirectionalLightComponent>(context, directionals);
    Builtin::RenderPipelineForward::ResourceBufferPointLights->update<PointLightComponent>(context, points);
    Builtin::RenderPipelineForward::ResourceBufferSpotLights->update<SpotLightComponent>(context, spots);

    // upload camera
    const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    Builtin::RenderPipelineForward::ResourceBufferCamera->update<Builtin::RenderPipelineForward::CameraBuffer>(
        context,
        {{
            .position = camera.getInfo().view.position,
            .view = camera.getView(),
            .projection = camera.getProjection(),
        }});

    // upload instance/object data
    Builtin::RenderPipelineForward::ResourceBufferModel->update<Builtin::RenderPipelineForward::ModelBuffer>(context, instanceBuffers);

    // record constants
    Builtin::RenderPipelineForward::PushConstants pushConstants = {
        .directionalCount = static_cast<std::uint32_t>(directionals.size()),
        .spotCount = static_cast<std::uint32_t>(spots.size()),
        .pointCount = static_cast<std::uint32_t>(points.size()),
        .materialBaseAddress = Builtin::RenderLayoutGlobal::ResourceBufferMaterials->getAddress(),
    };

    m_Info.pipeline->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

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

    // bind resources
    m_Info.pipeline->bind(context);
    context.systems.allocator->bind(context, m_Info.pipeline);

    // draw
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
