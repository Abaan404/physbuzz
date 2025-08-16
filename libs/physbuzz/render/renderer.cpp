#include "renderer.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../events/window.hpp"
#include "camera.hpp"

namespace Physbuzz {

namespace Builtin {

bool VertexRendererScreenQuad::build() {
    if (ResourceRegistry<VertexAttribute>::contains(Resource.getIdentifier())) {
        return true;
    }

    return ResourceRegistry<VertexAttribute>::insert(
        Resource.getIdentifier(),
        {{
            .attributes = {
                {
                    .type = Types::Float,
                    .size = sizeof(Format::position) / sizeof(decltype(Format::position)::value_type),
                    .offset = offsetof(Format, position),
                },
                {
                    .type = Types::Float,
                    .size = sizeof(Format::texCoords) / sizeof(decltype(Format::texCoords)::value_type),
                    .offset = offsetof(Format, texCoords),
                },
            },
            .size = sizeof(Format),
        }});
}

bool MeshRendererScreenQuad::build() {
    if (ResourceRegistry<Model>::contains(Resource.getIdentifier())) {
        return true;
    }

    if (!VertexRendererScreenQuad::build()) {
        return false;
    }

    return ResourceRegistry<Model>::insert(
        Resource.getIdentifier(),
        {{
            .meshes = {
                {
                    {
                        Mesh::Info<VertexRendererScreenQuad::Format>{
                            .attribute = {VertexRendererScreenQuad::Resource.getIdentifier()},
                            .vertices = {
                                {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
                                {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                                {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                            },
                            .indices = {0, 1, 2, 2, 3, 0},
                        },
                        {},
                    },
                },
            },
        }});
}

bool ShaderRendererPassthrough::build() {
    if (ResourceRegistry<ShaderPipeline>::contains(Resource.getIdentifier())) {
        return true;
    }

    if (!Builtin::MeshRendererScreenQuad::build()) {
        return false;
    }

    // return ResourceRegistry<ShaderPipeline>::insert(
    //     Resource.getIdentifier(),
    //     {{
    //         // .draw = [](const ShaderPipeline *, Scene &, ObjectID) {
    //         //     for (const auto &[mesh, _] : Builtin::MeshRendererScreenQuad::Resource->getMeshs()) {
    //         //         mesh.draw();
    //         //     }
    //         // },
    //     }});

    return true;
}

bool UniformRendererCamera::build() {
    if (ResourceRegistry<UniformBuffer<Format>>::contains(Resource.getIdentifier())) {
        return true;
    }

    bool success = ResourceRegistry<UniformBuffer<Format>>::insert(Resource.getIdentifier(), {});
    Resource->bindPipeline(Binding);

    return success;
}

} // namespace Builtin

Renderer::Renderer(const Info &info)
    : m_Info(info) {}

bool Renderer::build() {
    if (m_Command.pool != nullptr) {
        Logger::WARNING("[Renderer] Cannot build a constructed renderer.");
        return true;
    }

    if (!Builtin::UniformRendererCamera::build()) {
        Logger::ERROR("[Renderer] Could not create a constant camera buffer.");
        return false;
    }

    if (!Builtin::ShaderRendererPassthrough::build()) {
        return false;
    }

    // create command objects
    m_Command.pool = PBZ_VK_CHECK(App::Device.createCommandPool({
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = App::m_Indices.graphics,
    }));

    m_Command.buffers = PBZ_VK_CHECK(App::Device.allocateCommandBuffers({
        .commandPool = m_Command.pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = m_Command.maxFramesInFlight,
    }));

    // create sync objects
    for (std::size_t i = 0; i < m_Command.maxFramesInFlight; i++) {
        m_Semaphores.presentComplete.emplace_back(PBZ_VK_CHECK(App::Device.createSemaphore({})));
        m_Semaphores.renderFinished.emplace_back(PBZ_VK_CHECK(App::Device.createSemaphore({})));
        m_Fence.inFlight.emplace_back(PBZ_VK_CHECK(App::Device.createFence({
            .flags = vk::FenceCreateFlagBits::eSignaled,
        })));
    }

    // setup resize event
    m_Events = {
        .resize = m_Info.window->addCallback<WindowResizeEvent>([&](const auto &event) {
            resize(event.resolution);
        }),
    };

    // create renderer
    buildSystems();

    // create shadows
    m_Scene->createSystem<Shadow>(m_Info.shadow, m_Info.window->getResolution());

    return true;
}

bool Renderer::destroy() {
    // destroyRenderer(); // System::destroy() should not destroy other systems
    if (m_Command.pool == nullptr) {
        Logger::WARNING("[Renderer] Trying to destroy a destructed renderer");
        return true;
    }

    // destroy sync objects
    for (std::size_t i = 0; i < m_Command.maxFramesInFlight; i++) {
        App::Device.destroySemaphore(m_Semaphores.presentComplete[i]);
        App::Device.destroySemaphore(m_Semaphores.renderFinished[i]);

        App::Device.destroyFence(m_Fence.inFlight[i]);
    }

    m_Semaphores.renderFinished.clear();
    m_Semaphores.presentComplete.clear();
    m_Fence.inFlight.clear();

    // destroy command objects
    App::Device.freeCommandBuffers(m_Command.pool, m_Command.buffers.size(), m_Command.buffers.data());
    m_Command.buffers.clear();

    App::Device.destroyCommandPool(m_Command.pool);
    m_Command.pool = nullptr;

    return true;
}

void Renderer::tick() {
    while (vk::Result::eTimeout == App::Device.waitForFences(m_Fence.inFlight[m_Command.frameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max())) {
    }

    // fetch the next available swapchain image
    auto [acquireNextImageResult, imageIndex] = App::Device.acquireNextImageKHR(m_Info.window->m_SwapChain, std::numeric_limits<std::uint32_t>::max(), m_Semaphores.presentComplete[m_Command.frameInFlight], nullptr);

    switch (acquireNextImageResult) {
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
        break;

    case vk::Result::eErrorOutOfDateKHR:
        m_Info.window->recreateSwapChain();
        return;

    default:
        Logger::CRITICAL("[Renderer] Failed to acquire swap chain image.");
    }

    App::Device.resetFences(m_Fence.inFlight[m_Command.frameInFlight]);
    m_Command.buffers[m_Command.frameInFlight].reset();

    {
        vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
        vk::Result result = m_Command.buffers[m_Command.frameInFlight].begin(commandBufferBeginInfo);

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Rendering begin failed ({})", vk::to_string(result));
            return;
        }
    }

    // transition image
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
            .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_Info.window->m_SwapChainImages[imageIndex],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        m_Command.buffers[m_Command.frameInFlight].pipelineBarrier2(dependencyInfo);
    }

    // setup attachments
    std::vector<vk::RenderingAttachmentInfo> attachments = {
        {
            .imageView = m_Info.window->m_SwapChainImageViews[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
        },
    };

    // setup rendering
    glm::ivec2 resolution = m_Info.window->getResolution();

    m_Command.buffers[m_Command.frameInFlight].beginRendering({
        .renderArea = {
            .offset = {0, 0},
            .extent = {static_cast<std::uint32_t>(resolution.x), static_cast<std::uint32_t>(resolution.y)},
        },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<std::uint32_t>(attachments.size()),
        .pColorAttachments = attachments.data(),
    });

    switch (m_Info.type) {
    case Type::Deferred:
        m_Scene->tickSystem<Shadow, DeferredRenderer>();
        break;

    case Type::Forward:
        m_Scene->tickSystem<Shadow, ForwardRenderer>();
        break;

    default:
        Logger::ERROR("[Renderer] Unknown renderer type provided");
        return;
    }

    m_Command.buffers[m_Command.frameInFlight].endRendering();

    // transition image
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::eUndefined,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_Info.window->m_SwapChainImages[imageIndex],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        m_Command.buffers[m_Command.frameInFlight].pipelineBarrier2(dependencyInfo);
    }

    {
        vk::Result result = m_Command.buffers[m_Command.frameInFlight].end();

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Rendering end failed ({})", vk::to_string(result));
            return;
        }
    }

    // const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);
    // Builtin::UniformRendererCamera::Resource->update({
    //     .position = camera.getInfo().view.position,
    //     .view = camera.getView(),
    //     .projection = camera.getProjection(),
    // });

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_Semaphores.presentComplete[m_Command.frameInFlight],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffers[m_Command.frameInFlight],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_Semaphores.renderFinished[m_Command.frameInFlight],
    };

    {
        vk::Result result = App::GraphicsQueue.submit(submitInfo, m_Fence.inFlight[m_Command.frameInFlight]);

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Queue submission failed ({})", vk::to_string(result));
            return;
        }
    }

    {
        vk::Result result = App::PresentQueue.presentKHR({
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_Semaphores.renderFinished[m_Command.frameInFlight],
            .swapchainCount = 1,
            .pSwapchains = &m_Info.window->m_SwapChain,
            .pImageIndices = &imageIndex,
        });

        switch (result) {
        case vk::Result::eSuccess:
            break;
        case vk::Result::eErrorOutOfDateKHR:
        case vk::Result::eSuboptimalKHR:
            m_Info.window->recreateSwapChain();
            break;
        default:
            Logger::ERROR("[Renderer] Failed to present swapchain image! ({})", vk::to_string(result));
            return;
        }

        if (m_Info.window->m_FramebufferResized) {
            m_Info.window->m_FramebufferResized = false;
            m_Info.window->recreateSwapChain();
        }
    }

    m_Command.frameInFlight = (m_Command.frameInFlight + 1) % m_Command.maxFramesInFlight;
}

void Renderer::resize(const glm::ivec2 &resolution) {
    // if (!m_Scene->containsComponent<CameraComponent>(m_Info.camera)) {
    //     Logger::ERROR("[Renderer] No camera attached to object {}", m_Info.camera);
    //     return;
    // }
    //
    // const auto [camera] = m_Scene->getComponent<CameraComponent>(m_Info.camera);

    m_Scene->getSystem<Shadow>()->resize(resolution);
    getRenderer()->resize(resolution);
    // camera.resize(resolution);
}

void Renderer::setType(const Type &type) {
    destroySystems();
    m_Info.type = type;
    buildSystems();
}

const Renderer::Type &Renderer::getType() {
    return m_Info.type;
}

const Framebuffer &Renderer::getFramebuffer() const {
    return getRenderer()->getOutput();
}

const Renderer::Info &Renderer::getInfo() const {
    return m_Info;
}

std::shared_ptr<IRenderer> Renderer::getRenderer() const {
    switch (m_Info.type) {
    case Type::Deferred:
        return m_Scene->getSystem<DeferredRenderer>();

    case Type::Forward:
        return m_Scene->getSystem<ForwardRenderer>();
    }

    return nullptr;
}

bool Renderer::buildSystems() {
    switch (m_Info.type) {
    case Type::Deferred:
        m_Scene->createSystem<DeferredRenderer>(m_Info.deferred, m_Info.window->getResolution(), m_Command);
        break;

    case Type::Forward:
        m_Scene->createSystem<ForwardRenderer>(m_Info.forward, m_Info.window->getResolution(), m_Command);
        break;
    }

    return false;
}

bool Renderer::destroySystems() {
    switch (m_Info.type) {
    case Type::Deferred:
        return m_Scene->eraseSystem<DeferredRenderer>();

    case Type::Forward:
        return m_Scene->eraseSystem<ForwardRenderer>();
    }

    return false;
}

} // namespace Physbuzz
