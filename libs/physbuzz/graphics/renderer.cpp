#include "renderer.hpp"

#include "../app/application.hpp"
#include "layout.hpp"

namespace Physbuzz {

Renderer::Renderer(const Info &info, const RenderGraph &graph)
    : m_Info(info), m_Graph(graph) {}

bool Renderer::build() {
    if (m_Command.pool != nullptr) {
        Logger::WARNING("[Renderer] Cannot build a constructed renderer.");
        return true;
    }

    // create command objects
    m_Command.pool = PBZ_VK_CHECK(App::Device.createCommandPool({
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = App::Indices.graphics,
    }));

    vk::CommandBufferAllocateInfo allocateInfo = {
        .commandPool = m_Command.pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = detail::MAX_FRAMES_IN_FLIGHT,
    };

    PBZ_VK_CHECK_RESULT(App::Device.allocateCommandBuffers(&allocateInfo, m_Command.buffers.begin()));

    // create sync objects
    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        m_Semaphores.presentComplete[i] = PBZ_VK_CHECK(App::Device.createSemaphore({}));
        m_Fences.inFlight[i] = (PBZ_VK_CHECK(App::Device.createFence({
            .flags = vk::FenceCreateFlagBits::eSignaled,
        })));
    }

    for (std::size_t i = 0; i < m_Info.window->m_SwapChainImages.size(); i++) {
        m_Semaphores.renderFinished.emplace_back(PBZ_VK_CHECK(App::Device.createSemaphore({})));
    }

    // setup depth buffer
    if (!m_Depth.build(m_Info.window->getResolution())) {
        Logger::ERROR("[Renderer] Could not build a depth buffer.");
        destroy();
        return false;
    }

    // setup material handler
    if (!m_MaterialAllocator.build()) {
        Logger::ERROR("[Renderer] Could not create the materials manager.");
        destroy();
        return false;
    }

    return true;
}

bool Renderer::destroy() {
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.inFlight[m_FrameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max()));

    if (m_Command.pool == nullptr) {
        Logger::WARNING("[Renderer] Trying to destroy a destructed renderer.");
        return true;
    }

    // destroy global sets
    m_MaterialAllocator.destroy();

    // destroy depth buffer
    if (!m_Depth.destroy()) {
        Logger::ERROR("[Renderer] Failed to destroy depth buffer.");
        return false;
    }

    // destroy sync objects
    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        m_DeletionQueues[i].flush();
        App::Device.destroySemaphore(m_Semaphores.presentComplete[i]);
        App::Device.destroyFence(m_Fences.inFlight[i]);
    }

    for (std::size_t i = 0; i < m_Info.window->m_SwapChainImages.size(); i++) {
        App::Device.destroySemaphore(m_Semaphores.renderFinished[i]);
    }

    m_Semaphores.renderFinished.clear();
    m_Semaphores.presentComplete.fill(nullptr);
    m_Fences.inFlight.fill(nullptr);

    // destroy command objects
    App::Device.freeCommandBuffers(m_Command.pool, m_Command.buffers.size(), m_Command.buffers.data());
    m_Command.buffers.fill(nullptr);

    App::Device.destroyCommandPool(m_Command.pool);
    m_Command.pool = nullptr;

    return true;
}

void Renderer::tick() {
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.inFlight[m_FrameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max()));
    m_DeletionQueues[m_FrameInFlight].flush();

    // fetch the next available swapchain image
    auto [acquireNextImageResult, imageIndex] = App::Device.acquireNextImageKHR(m_Info.window->m_SwapChain, std::numeric_limits<std::uint32_t>::max(), m_Semaphores.presentComplete[m_FrameInFlight], nullptr);

    switch (acquireNextImageResult) {
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
        break;

    case vk::Result::eErrorOutOfDateKHR:
        m_Info.window->recreateSwapChain();
        return;

    default:
        PBZ_VK_CHECK_RESULT(acquireNextImageResult, "[Renderer] Failed to acquire swap chain image.");
    }

    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.inFlight[m_FrameInFlight]));
    PBZ_VK_CHECK_RESULT(m_Command.buffers[m_FrameInFlight].reset());

    PBZ_VK_CHECK_RESULT(m_Command.buffers[m_FrameInFlight].begin(vk::CommandBufferBeginInfo{}));

    vk::Extent2D extent = {static_cast<std::uint32_t>(m_Info.window->m_SwapChainExtent.x), static_cast<std::uint32_t>(m_Info.window->m_SwapChainExtent.y)};

    m_Command.buffers[m_FrameInFlight].setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f});
    m_Command.buffers[m_FrameInFlight].setScissor(0, vk::Rect2D{{0, 0}, extent});

    RenderContext context = {
        .deletionQueue = &m_DeletionQueues[m_FrameInFlight],
        .materialAllocator = &m_MaterialAllocator,
        .depth = &m_Depth,
        .command = m_Command.buffers[m_FrameInFlight],
        .extent = extent,
        .frameInFlight = m_FrameInFlight,
        .color = {
            .image = m_Info.window->m_SwapChainImages[imageIndex],
            .view = m_Info.window->m_SwapChainImageViews[imageIndex],
        },
    };

    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .image = m_Info.window->m_SwapChainImages[imageIndex],
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            },
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                .image = m_Depth.getRingData()[m_FrameInFlight].image.getData().image,
                .subresourceRange = m_Depth.getRingData()[m_FrameInFlight].subresourceRange,
            },
        };

        m_Command.buffers[m_FrameInFlight].pipelineBarrier2({
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    glm::uvec2 resolution = m_Depth.getSize(m_FrameInFlight);
    if (resolution.x != extent.width || resolution.y != extent.height) {
        m_Depth.rebuild(context, {extent.width, extent.height});
    }

    // execute the graph
    m_Graph.execute(m_Scene, context);

    // refresh the material state
    m_MaterialAllocator.refresh(context);

    // transition image
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eNone,
            .dstAccessMask = vk::AccessFlagBits2::eNone,
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::ePresentSrcKHR,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
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

        m_Command.buffers[m_FrameInFlight].pipelineBarrier2(dependencyInfo);
    }

    PBZ_VK_CHECK_RESULT(m_Command.buffers[m_FrameInFlight].end());

    vk::PipelineStageFlags waitDestinationStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    const vk::SubmitInfo submitInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_Semaphores.presentComplete[m_FrameInFlight],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffers[m_FrameInFlight],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_Semaphores.renderFinished[imageIndex],
    };

    PBZ_VK_CHECK_RESULT(App::Queues.graphics.submit(submitInfo, m_Fences.inFlight[m_FrameInFlight]));

    vk::Result presentResult = App::Queues.present.presentKHR({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_Semaphores.renderFinished[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &m_Info.window->m_SwapChain,
        .pImageIndices = &imageIndex,
    });

    switch (presentResult) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eErrorOutOfDateKHR:
    case vk::Result::eSuboptimalKHR:
        m_Info.window->recreateSwapChain();
        break;
    default:
        PBZ_VK_CHECK_RESULT(presentResult, "[Renderer] Failed to present swapchain image!");
        return;
    }

    if (m_Info.window->m_FramebufferResized) {
        m_Info.window->m_FramebufferResized = false;
        m_Info.window->recreateSwapChain();
    }

    m_FrameInFlight = (m_FrameInFlight + 1) % detail::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::immediate(std::function<void(vk::CommandBuffer)> record) {
    // prepare the command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.inFlight[m_FrameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.inFlight[m_FrameInFlight]));
    PBZ_VK_CHECK_RESULT(m_Command.buffers[m_FrameInFlight].reset());

    PBZ_VK_CHECK_RESULT(m_Command.buffers[m_FrameInFlight].begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    record(m_Command.buffers[m_FrameInFlight]);

    PBZ_VK_CHECK_RESULT(m_Command.buffers[m_FrameInFlight].end());

    vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffers[m_FrameInFlight],
    };

    PBZ_VK_CHECK_RESULT(App::Queues.graphics.submit(submitInfo, m_Fences.inFlight[m_FrameInFlight]));
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.inFlight[m_FrameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max()));

    m_FrameInFlight = (m_FrameInFlight + 1) % detail::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::setGraph(const RenderGraph &graph) {
    m_Graph = graph;
}

const RenderGraph &Renderer::getGraph() const {
    return m_Graph;
}

const Renderer::Info &Renderer::getInfo() const {
    return m_Info;
}

// create a custom ImGui_Physbuzz_Impl so this shouldnt be exposed anymore
std::uint32_t Renderer::getFrameInFlight() const {
    return m_FrameInFlight;
}

} // namespace Physbuzz
