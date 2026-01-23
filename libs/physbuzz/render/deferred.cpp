#include "deferred.hpp"

#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "../graphics/descriptors/dynamic.hpp"
#include "../graphics/descriptors/static.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/renderer.hpp"
#include "camera.hpp"
#include "resources/common.hpp"

namespace Physbuzz {

DeferredRenderer::DeferredRenderer(const Info &info)
    : m_Info(info) {}

bool DeferredRenderer::build() {
    bool success = true;

    // build pipeline
    if (m_Info.geometry == Builtin::RenderPipelineDeferred::ResourceGeometry || m_Info.lighting == Builtin::RenderPipelineDeferred::ResourceLighting) {
        if (!Builtin::RenderLayoutGlobal::build()) {
            Logger::ERROR("[Renderer] Could not build global layouts.");
            return false;
        }

        if (!Builtin::RenderPipelineDeferred::build()) {
            Logger::ERROR("[Renderer] Could not build deferred shader pipelines.");
            return false;
        }
    }

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineDeferred::ResourceLayoutFrame,
        Builtin::RenderPipelineDeferred::ResourceBufferCamera,
        0);

    success &= m_Scene->getSystem<PipelineLayoutAllocator>()->attach(
        Builtin::RenderPipelineDeferred::ResourceLayoutObject,
        Builtin::RenderPipelineDeferred::ResourceBufferModel,
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

bool DeferredRenderer::destroy() {
    m_Scene->getSystem<Renderer>()->getInfo().window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);

    return true;
}

void DeferredRenderer::render(const RenderContext &context) {
    // batch objects
    std::unordered_map<Resource<Mesh>, std::vector<Builtin::RenderPipelineDeferred::ModelBuffer>> instances;

    std::size_t meshCount = 0;
    for (const auto &object : m_Objects) {
        const auto [render] = m_Scene->getComponent<RenderComponent>(object);

        Builtin::RenderLayoutGlobal::refresh(context, render.model);

        const Model::Info &model = render.model.getInfo();
        for (const auto &mesh : model.meshes) {
            instances[mesh.mesh].emplace_back<Builtin::RenderPipelineDeferred::ModelBuffer>({
                .model = render.transform.matrix,
                .invModel = glm::inverse(render.transform.matrix),
                .materialIdx = Builtin::RenderLayoutGlobal::TableMaterial.query(mesh.material),
            });
        }

        meshCount += render.model.getInfo().meshes.size();
    }

    std::vector<std::pair<Resource<Mesh>, std::size_t>> instanceSizes;
    std::vector<Builtin::RenderPipelineDeferred::ModelBuffer> instanceBuffers;

    instanceBuffers.reserve(meshCount);
    instanceSizes.reserve(instances.size());

    for (const auto &[mesh, buffers] : instances) {
        instanceSizes.emplace_back<std::pair<Resource<Mesh>, std::size_t>>({mesh, buffers.size()});
        instanceBuffers.insert(instanceBuffers.end(), std::make_move_iterator(buffers.begin()), std::make_move_iterator(buffers.end()));
    }

    std::size_t requiredModelSize = meshCount * sizeof(Builtin::RenderPipelineDeferred::ModelBuffer);
    if (Builtin::RenderPipelineDeferred::ResourceBufferModel->getSize(context) < requiredModelSize) {
        Builtin::RenderPipelineDeferred::ResourceBufferModel->resize(context, requiredModelSize);

        // retach for a new buffer
        context.systems.allocator->reattach(
            context,
            Builtin::RenderPipelineDeferred::ResourceLayoutObject,
            Builtin::RenderPipelineDeferred::ResourceBufferModel,
           0);
    }

    for (const auto &gBuffer : Builtin::RenderPipelineDeferred::ResourceTextureGBuffers) {
        glm::uvec3 resolution = gBuffer->getSize();
        if (resolution.x != context.extent.width || resolution.y != context.extent.height) {
            gBuffer->resize(context, {context.extent.width, context.extent.height, 1});
        }
    }

    // upload camera
    const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    Builtin::RenderPipelineDeferred::ResourceBufferCamera->update<Builtin::RenderPipelineDeferred::CameraBuffer>(
        context,
        {{
            .view = camera.getView(),
            .projection = camera.getProjection(),
        }});

    // upload instance/object data
    Builtin::RenderPipelineDeferred::ResourceBufferModel->update<Builtin::RenderPipelineDeferred::ModelBuffer>(context, instanceBuffers);

    // record constants
    Builtin::RenderPipelineDeferred::PushConstants pushConstants = {
        .materialBaseAddress = Builtin::RenderLayoutGlobal::ResourceBufferMaterials->getAddress(),
    };

    m_Info.geometry->updatePushConstants(context, RenderPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

    // setup attachments and barriers
    std::array<vk::RenderingAttachmentInfo, Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size() + 1> colorAttachments;
    std::array<vk::ImageMemoryBarrier2, Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size()> barriers;

    for (std::size_t i = 0; i < Builtin::RenderPipelineDeferred::ResourceTextureGBuffers.size(); i++) {
        colorAttachments[i] = {
            .imageView = Builtin::RenderPipelineDeferred::ResourceTextureGBuffers[i]->getData().view,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
        };

        barriers[i] = {
            .srcStageMask = vk::PipelineStageFlagBits2::eNone,
            .srcAccessMask = vk::AccessFlagBits2::eNone,
            .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .image = Builtin::RenderPipelineDeferred::ResourceTextureGBuffers[i]->getImage().getData().image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
    }

    // REMOVE ME
    colorAttachments[3] = {
        .imageView = context.color.view,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
    };

    vk::RenderingAttachmentInfo depthAttachment = {
        .imageView = context.depth.view,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
    };

    context.command.pipelineBarrier2({
        .imageMemoryBarrierCount = barriers.size(),
        .pImageMemoryBarriers = barriers.data(),
    });

    // geometry pass
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
    m_Info.geometry->bind(context);
    context.systems.allocator->bind(context, m_Info.geometry);

    std::uint32_t object = 0;
    for (const auto &[mesh, batch] : instanceSizes) {
        if (mesh->getDescription() != m_Info.geometry->getInfo().description) {
            Logger::ERROR("[ForwardRenderer] Incompatible vertex state descriptions.");
            continue;
        }

        mesh->draw(context, batch, object);
        object += batch;
    }

    context.command.endRendering();

    // // lighting pass
    // context.command.beginRendering({
    //     .renderArea = {
    //         .offset = {0, 0},
    //         .extent = context.extent,
    //     },
    //     .layerCount = 1,
    //     .colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments.size()),
    //     .pColorAttachments = colorAttachments.data(),
    //     .pDepthAttachment = &depthAttachment,
    //     .pStencilAttachment = {},
    // });
    //
    // // bind resources
    // m_Info.geometry->bind(context);
    // context.systems.allocator->bind(context, m_Info.geometry);
    //
    // // draw one triangle
    // context.command.drawIndexed(3, 1, 0, 0, 0);
    //
    // context.command.endRendering();

    // // check for reloads before rendering
    // if (!m_Info.passes.geometry->reload()) {
    //     return;
    // }
    //
    // if (!m_Info.passes.lighting->reload()) {
    //     return;
    // }
    //
    // // geometry pass
    // m_Framebuffers.gBuffer.bind();
    // m_Framebuffers.gBuffer.clear();
    //
    // // render to gBuffers
    // for (const auto &object : m_Objects) {
    //     render(object);
    // }
    //
    // // lighting pass
    // m_Framebuffers.output.bind();
    // m_Framebuffers.output.clear();
    //
    // m_Info.passes.lighting->bind();
    //
    // for (std::size_t i = 0; i < m_Framebuffers.gBuffer.getInfo().colors.size(); i++) {
    //     m_Info.passes.lighting->setUniform(
    //         std::format("PBZ_GBuffer{}", i),
    //         m_Framebuffers.gBuffer.activate(Framebuffer::Type::Color, i));
    // }
    //
    // m_Info.passes.lighting->draw(*m_Scene, -1);
    // m_Info.passes.lighting->unbind();
    //
    // m_Framebuffers.output.blit(
    //     m_Framebuffers.gBuffer,
    //     {{0, 0}, m_Framebuffers.output.getInfo().resolution},
    //     {{0, 0}, m_Framebuffers.output.getInfo().resolution},
    //     Framebuffer::Mask::Depth);
    //
    // // forward passes
    // for (const auto [object, forward] : m_Scene->getComponents<DeferredRenderComponent::ForwardPass>()) {
    //     if (!forward.pipeline->reload()) {
    //         continue;
    //     }
    //
    //     forward.pipeline->bind();
    //     forward.pipeline->draw(*m_Scene, object);
    //     forward.pipeline->unbind();
    // }
    //
    // m_Framebuffers.output.unbind();
}

// void DeferredRenderer::render(ObjectID object) const {
// const auto [render] = m_Scene->getComponent<RenderComponent>(object);
//
// GL::detail::TextureUnits::reset();
// m_Info.passes.geometry->bind();
// m_Info.passes.geometry->draw(*m_Scene, object);
// m_Info.passes.geometry->unbind();
// }

const DeferredRenderer::Info &DeferredRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
