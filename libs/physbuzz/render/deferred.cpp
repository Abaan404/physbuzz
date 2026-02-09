#include "deferred.hpp"

#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/descriptors/dynamic.hpp"
#include "../graphics/descriptors/static.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/renderer.hpp"
#include "camera.hpp"
#include "lighting.hpp"

namespace Physbuzz {

DeferredRenderer::DeferredRenderer(const Info &info)
    : m_Info(info) {}

bool DeferredRenderer::build() {
    bool success = true;

    // build pipeline
    if (m_Info.geometry == Builtin::RenderPipelineDeferred::Geometry::Resource || m_Info.lighting == Builtin::RenderPipelineDeferred::Lighting::Resource) {
        if (!Builtin::RenderPipelineDeferred::build()) {
            Logger::ERROR("[Renderer] Could not build deferred shader pipelines.");
            return false;
        }
    }

    // geometry
    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
        Builtin::RenderPipelineDeferred::Geometry::ResourceLayoutFrame,
        Builtin::RenderPipelineDeferred::ResourceBufferCamera,
        0);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
        Builtin::RenderPipelineDeferred::Geometry::ResourceLayoutObject,
        Builtin::RenderPipelineDeferred::Geometry::ResourceBufferModel,
        0);

    // lighting
    for (std::size_t i = 0; i < Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size(); i++) {
        success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
            Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutGBuffer,
            Builtin::RenderPipelineDeferred::ResourceTextureGBuffers[i],
            i);
    }

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
        Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
        Builtin::RenderPipelineDeferred::ResourceBufferCamera,
        0);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
        Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
        Builtin::RenderPipelineDeferred::Lighting::ResourceBufferDirectionalLights,
        1);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
        Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
        Builtin::RenderPipelineDeferred::Lighting::ResourceBufferPointLights,
        2);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->write(
        Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
        Builtin::RenderPipelineDeferred::Lighting::ResourceBufferSpotLights,
        3);

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

bool DeferredRenderer::destroy() {
    m_Scene->getSystem<Renderer>()->getInfo().window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);

    return true;
}

void DeferredRenderer::render(const RenderContext &context) {
    // // batch objects
    // std::unordered_map<Resource<Mesh>, std::vector<Builtin::RenderPipelineDeferred::Geometry::ModelBuffer>> instances;
    //
    // std::size_t meshCount = 0;
    // for (const auto &object : m_Objects) {
    //     const auto [render] = m_Scene->getComponent<RenderComponent>(object);
    //
    //     Builtin::RenderLayoutGlobal::refresh(context, render.model);
    //
    //     const Model::Info &model = render.model.getInfo();
    //     for (const auto &mesh : model.meshes) {
    //         instances[mesh.mesh].emplace_back<Builtin::RenderPipelineDeferred::Geometry::ModelBuffer>({
    //             .model = render.transform.matrix,
    //             .invModel = glm::inverse(render.transform.matrix),
    //             .materialIdx = Builtin::RenderLayoutGlobal::TableMaterial.query(mesh.material),
    //         });
    //     }
    //
    //     meshCount += render.model.getInfo().meshes.size();
    // }
    //
    // std::vector<std::pair<Resource<Mesh>, std::size_t>> instanceSizes;
    // std::vector<Builtin::RenderPipelineDeferred::Geometry::ModelBuffer> instanceBuffers;
    //
    // instanceBuffers.reserve(meshCount);
    // instanceSizes.reserve(instances.size());
    //
    // for (const auto &[mesh, buffers] : instances) {
    //     instanceSizes.emplace_back<std::pair<Resource<Mesh>, std::size_t>>({mesh, buffers.size()});
    //     instanceBuffers.insert(instanceBuffers.end(), std::make_move_iterator(buffers.begin()), std::make_move_iterator(buffers.end()));
    // }
    //
    // std::size_t requiredModelSize = meshCount * sizeof(Builtin::RenderPipelineDeferred::Geometry::ModelBuffer);
    // if (Builtin::RenderPipelineDeferred::Geometry::ResourceBufferModel->getSize(context) < requiredModelSize) {
    //     Builtin::RenderPipelineDeferred::Geometry::ResourceBufferModel->resize(context, requiredModelSize);
    //
    //     // retach for a new buffer
    //     context.systems.allocator->rewrite(
    //         context,
    //         Builtin::RenderPipelineDeferred::Geometry::ResourceLayoutObject,
    //         Builtin::RenderPipelineDeferred::Geometry::ResourceBufferModel,
    //         0);
    // }
    //
    // for (const auto &gBuffer : Builtin::RenderPipelineDeferred::ResourceTextureGBuffers) {
    //     glm::uvec3 resolution = gBuffer->getSize();
    //     if (resolution.x != context.extent.width || resolution.y != context.extent.height) {
    //         gBuffer->resize(context, {context.extent.width, context.extent.height, 1});
    //
    //         for (std::size_t i = 0; i < Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size(); i++) {
    //             m_Scene->getSystem<PipelineLayoutAllocator>()->write(
    //                 Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutGBuffer,
    //                 Builtin::RenderPipelineDeferred::ResourceTextureGBuffers[i],
    //                 i);
    //         }
    //     }
    // }
    //
    // // upload camera
    // const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    // Builtin::RenderPipelineDeferred::ResourceBufferCamera->update<Builtin::RenderPipelineDeferred::CameraBuffer>(
    //     context,
    //     {{
    //         .position = camera.getInfo().view.position,
    //         .view = camera.getView(),
    //         .projection = camera.getProjection(),
    //     }});
    //
    // // upload instance/object data
    // Builtin::RenderPipelineDeferred::Geometry::ResourceBufferModel->update<Builtin::RenderPipelineDeferred::Geometry::ModelBuffer>(context, instanceBuffers);
    //
    // // upload lighting data
    // const std::vector<DirectionalLightComponent> &directionals = m_Scene->getComponentArray<DirectionalLightComponent>();
    // const std::vector<PointLightComponent> &points = m_Scene->getComponentArray<PointLightComponent>();
    // const std::vector<SpotLightComponent> &spots = m_Scene->getComponentArray<SpotLightComponent>();
    //
    // std::size_t requiredDirectionalSize = directionals.size() * sizeof(DirectionalLightComponent);
    // if (Builtin::RenderPipelineDeferred::Lighting::ResourceBufferDirectionalLights->getSize(context) < requiredDirectionalSize) {
    //     Builtin::RenderPipelineDeferred::Lighting::ResourceBufferDirectionalLights->resize(context, requiredDirectionalSize);
    //
    //     // retach for a new buffer
    //     context.systems.allocator->rewrite(
    //         context,
    //         Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
    //         Builtin::RenderPipelineDeferred::Lighting::ResourceBufferDirectionalLights,
    //         1);
    // }
    //
    // std::size_t requiredPointsSize = points.size() * sizeof(PointLightComponent);
    // if (Builtin::RenderPipelineDeferred::Lighting::ResourceBufferPointLights->getSize(context) < requiredPointsSize) {
    //     Builtin::RenderPipelineDeferred::Lighting::ResourceBufferPointLights->resize(context, requiredPointsSize);
    //
    //     // retach for a new buffer
    //     context.systems.allocator->rewrite(
    //         context,
    //         Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
    //         Builtin::RenderPipelineDeferred::Lighting::ResourceBufferPointLights,
    //         2);
    // }
    //
    // std::size_t requiredSpotSize = spots.size() * sizeof(SpotLightComponent);
    // if (Builtin::RenderPipelineDeferred::Lighting::ResourceBufferSpotLights->getSize(context) < requiredSpotSize) {
    //     Builtin::RenderPipelineDeferred::Lighting::ResourceBufferSpotLights->resize(context, requiredSpotSize);
    //
    //     // retach for a new buffer
    //     context.systems.allocator->rewrite(
    //         context,
    //         Builtin::RenderPipelineDeferred::Lighting::ResourceLayoutFrame,
    //         Builtin::RenderPipelineDeferred::Lighting::ResourceBufferSpotLights,
    //         3);
    // }
    //
    // Builtin::RenderPipelineDeferred::Lighting::ResourceBufferDirectionalLights->update<DirectionalLightComponent>(context, directionals);
    // Builtin::RenderPipelineDeferred::Lighting::ResourceBufferPointLights->update<PointLightComponent>(context, points);
    // Builtin::RenderPipelineDeferred::Lighting::ResourceBufferSpotLights->update<SpotLightComponent>(context, spots);
    //
    // // geometry pass
    // std::array<vk::RenderingwritementInfo, Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size() + 1> colorwritements;
    // std::array<vk::ImageMemoryBarrier2, Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size()> gBufferLayoutBarriers;
    //
    // for (std::size_t i = 0; i < Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size(); i++) {
    //     colorwritements[i + 1] = {
    //         .imageView = Builtin::RenderPipelineDeferred::ResourceTextureGBuffers[i]->getData().view,
    //         .imageLayout = vk::ImageLayout::eColorwritementOptimal,
    //         .loadOp = vk::writementLoadOp::eClear,
    //         .storeOp = vk::writementStoreOp::eStore,
    //         .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
    //     };
    //
    //     // TODO: make this immediate at creation?
    //     gBufferLayoutBarriers[i] = {
    //         .srcStageMask = vk::PipelineStageFlagBits2::eNone,
    //         .srcAccessMask = vk::AccessFlagBits2::eNone,
    //         .dstStageMask = vk::PipelineStageFlagBits2::eColorwritementOutput,
    //         .dstAccessMask = vk::AccessFlagBits2::eColorwritementWrite,
    //         .oldLayout = vk::ImageLayout::eRenderingLocalRead,
    //         .newLayout = vk::ImageLayout::eRenderingLocalRead,
    //         .image = Builtin::RenderPipelineDeferred::ResourceTextureGBuffers[i]->getImage().getData().image,
    //         .subresourceRange = {
    //             .aspectMask = vk::ImageAspectFlagBits::eColor,
    //             .baseMipLevel = 0,
    //             .levelCount = 1,
    //             .baseArrayLayer = 0,
    //             .layerCount = 1,
    //         },
    //     };
    // }
    //
    // colorwritements[0] = {
    //     .imageView = context.color.view,
    //     .imageLayout = vk::ImageLayout::eColorwritementOptimal,
    //     .loadOp = vk::writementLoadOp::eClear,
    //     .storeOp = vk::writementStoreOp::eStore,
    //     .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
    // };
    //
    // vk::RenderingwritementInfo depthwritement = {
    //     .imageView = context.depth.view,
    //     .imageLayout = vk::ImageLayout::eDepthwritementOptimal,
    //     .loadOp = vk::writementLoadOp::eClear,
    //     .storeOp = vk::writementStoreOp::eDontCare,
    //     .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
    // };
    //
    // context.command.pipelineBarrier2({
    //     .imageMemoryBarrierCount = gBufferLayoutBarriers.size(),
    //     .pImageMemoryBarriers = gBufferLayoutBarriers.data(),
    // });
    //
    // // issue draw calls
    // context.command.beginRendering({
    //     .renderArea = {
    //         .offset = {0, 0},
    //         .extent = context.extent,
    //     },
    //     .layerCount = 1,
    //     .colorwritementCount = static_cast<std::uint32_t>(colorwritements.size()),
    //     .pColorwritements = colorwritements.data(),
    //     .pDepthwritement = &depthwritement,
    //     .pStencilwritement = {},
    // });
    //
    // std::array inputIndices = {vk::writementUnused, 0u, 1u, 2u};
    //
    // context.command.setRenderingInputwritementIndices({
    //     .colorwritementCount = 4,
    //     .pColorwritementInputIndices = inputIndices.data(),
    // });
    //
    // // bind resources
    // m_Info.geometry->bind(context);
    // context.systems.allocator->bind(context, m_Info.geometry);
    //
    // // record constants
    // Builtin::RenderPipelineDeferred::Geometry::PushConstants pushConstantsGeometry = {
    //     .materialBaseAddress = Builtin::RenderLayoutGlobal::ResourceBufferMaterials->getAddress(),
    // };
    //
    // m_Info.geometry->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstantsGeometry, 1)), 0);
    //
    // std::uint32_t object = 0;
    // for (const auto &[mesh, batch] : instanceSizes) {
    //     if (mesh->getDescription() != m_Info.geometry->getInfo().description) {
    //         Logger::ERROR("[ForwardRenderer] Incompatible vertex state descriptions.");
    //         continue;
    //     }
    //
    //     mesh->draw(context, batch, object);
    //     object += batch;
    // }
    //
    // std::array<vk::MemoryBarrier2, Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size()> gBufferSubpassBarriers;
    //
    // for (std::size_t i = 0; i < Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size(); i++) {
    //     gBufferSubpassBarriers[i] = {
    //         .srcStageMask = vk::PipelineStageFlagBits2::eColorwritementOutput,
    //         .srcAccessMask = vk::AccessFlagBits2::eColorwritementWrite,
    //         .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
    //         .dstAccessMask = vk::AccessFlagBits2::eColorwritementRead,
    //     };
    // }
    //
    // context.command.pipelineBarrier2({
    //     .dependencyFlags = vk::DependencyFlagBits::eByRegion,
    //     .memoryBarrierCount = gBufferSubpassBarriers.size(),
    //     .pMemoryBarriers = gBufferSubpassBarriers.data(),
    // });
    //
    // // bind resources
    // m_Info.lighting->bind(context);
    // context.systems.allocator->bind(context, m_Info.lighting);
    //
    // // record constants
    // Builtin::RenderPipelineDeferred::Lighting::PushConstants pushConstantsLighting = {
    //     .directionalCount = static_cast<std::uint32_t>(directionals.size()),
    //     .spotCount = static_cast<std::uint32_t>(spots.size()),
    //     .pointCount = static_cast<std::uint32_t>(points.size()),
    // };
    //
    // m_Info.lighting->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstantsLighting, 1)), 0);
    //
    // // draw one triangle
    // context.command.draw(3, 1, 0, 0);
    //
    // context.command.endRendering();
}

const DeferredRenderer::Info &DeferredRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
