#include "renderer.hpp"

#include "../app/application.hpp"
#include "../events/window.hpp"

namespace Physbuzz {

Renderer::Renderer(const Info &info)
    : m_Info(info) {}

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
    glm::uvec2 resolution = m_Info.window->getResolution();
    if (!m_Depth.image.build({resolution.x, resolution.y, 1})) {
        Logger::ERROR("[Renderer] Could not build a renderer depth buffer.");
        return false;
    }

    m_Depth.view = PBZ_VK_CHECK(App::Device.createImageView({
        .flags = {},
        .image = m_Depth.image.getData().image,
        .viewType = vk::ImageViewType::e2D,
        .format = m_Depth.image.getInfo().format,
        .components = {},
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    }));

    m_Events = {
        .resize = m_Info.window->addCallback<WindowSwapchainResizeEvent>([&](const WindowSwapchainResizeEvent &event) {
            resize(event.resolution);
        }),
    };

    return true;
}

bool Renderer::destroy() {
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.inFlight[m_FrameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max()));

    if (m_Command.pool == nullptr) {
        Logger::WARNING("[Renderer] Trying to destroy a destructed renderer.");
        return true;
    }

    // destroy depth buffer
    if (!m_Depth.image.destroy()) {
        Logger::ERROR("[Renderer] Failed to destroy depth buffer.");
        return false;
    }

    App::Device.destroyImageView(m_Depth.view);

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
    m_Command.buffers[m_FrameInFlight].reset();

    PBZ_VK_CHECK_RESULT(m_Command.buffers[m_FrameInFlight].begin(vk::CommandBufferBeginInfo{}));

    vk::Extent2D extent = {static_cast<std::uint32_t>(m_Info.window->m_SwapChainExtent.x), static_cast<std::uint32_t>(m_Info.window->m_SwapChainExtent.y)};

    m_Command.buffers[m_FrameInFlight].setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f});
    m_Command.buffers[m_FrameInFlight].setScissor(0, vk::Rect2D{{0, 0}, extent});

    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
                .srcAccessMask = {},
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
                .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
                .srcAccessMask = {},
                .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests,
                .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                .image = m_Depth.image.getData().image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eDepth,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            },
        };

        m_Command.buffers[m_FrameInFlight].pipelineBarrier2({
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    for (const auto &renderpasses : m_RenderPasses) {
        renderpasses->render({
            .deletionQueue = &m_DeletionQueues[m_FrameInFlight],
            .command = m_Command.buffers[m_FrameInFlight],
            .extent = extent,
            .frameInFlight = m_FrameInFlight,
            .color = {
                .image = m_Info.window->m_SwapChainImages[imageIndex],
                .view = m_Info.window->m_SwapChainImageViews[imageIndex],
            },
            .depth = {
                .image = m_Depth.image.getData().image,
                .view = m_Depth.view,
            },
        });

        {
            std::array barriers = {
                vk::ImageMemoryBarrier2{
                    .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                    .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                    .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                    .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                    .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
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
                    .srcStageMask = vk::PipelineStageFlagBits2::eLateFragmentTests,
                    .srcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                    .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests,
                    .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                    .oldLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                    .newLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                    .image = m_Depth.image.getData().image,
                    .subresourceRange = {
                        .aspectMask = vk::ImageAspectFlagBits::eDepth,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                },
            };

            m_Command.buffers[m_FrameInFlight].pipelineBarrier2({
                .imageMemoryBarrierCount = barriers.size(),
                .pImageMemoryBarriers = barriers.data(),
            });
        }
    }

    // transition image
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
            .dstAccessMask = {},
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

void Renderer::immediate(std::function<void(const vk::CommandBuffer &)> record) {
    // prepare the command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.inFlight[m_FrameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.inFlight[m_FrameInFlight]));
    m_Command.buffers[m_FrameInFlight].reset();

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

void Renderer::setRenderPasses(const std::vector<std::shared_ptr<IRenderPass>> &renderpasses) {
    m_RenderPasses = renderpasses;
}

const Renderer::Info &Renderer::getInfo() const {
    return m_Info;
}

std::uint32_t Renderer::getFrameInFlight() const {
    return m_FrameInFlight;
}

void Renderer::resize(const glm::ivec2 &resolution) {
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.inFlight[m_FrameInFlight], vk::True, std::numeric_limits<std::uint64_t>::max()));

    if (!m_Depth.image.destroy()) {
        Logger::WARNING("[ForwardRenderer] Could not destroy old depth image");
    }

    App::Device.destroyImageView(m_Depth.view);

    if (!m_Depth.image.build({resolution, 1})) {
        Logger::ERROR("[ForwardRenderer] Could not rebuild depth image.");
    }

    m_Depth.view = PBZ_VK_CHECK(App::Device.createImageView({
        .flags = {},
        .image = m_Depth.image.getData().image,
        .viewType = vk::ImageViewType::e2D,
        .format = m_Depth.image.getInfo().format,
        .components = {},
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    }));
}

} // namespace Physbuzz
