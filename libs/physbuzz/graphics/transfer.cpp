#include "transfer.hpp"

#include "../app/application.hpp"

namespace Physbuzz {

Transfer::Transfer()
    : m_Deletion({}) {}

bool Transfer::build() {
    m_Command.pool = PBZ_VK_CHECK(App::Device.createCommandPool({
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = App::Indices.transfer,
    }));

    std::vector<vk::CommandBuffer> buffers = PBZ_VK_CHECK(App::Device.allocateCommandBuffers({
        .commandPool = m_Command.pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    }));

    m_Command.buffer = buffers.front();

    m_Fences.submit = PBZ_VK_CHECK(App::Device.createFence({
        .flags = vk::FenceCreateFlagBits::eSignaled,
    }));

    return true;
}

bool Transfer::destroy() {
    App::Device.destroyFence(m_Fences.submit);

    App::Device.freeCommandBuffers(m_Command.pool, 1, &m_Command.buffer);
    m_Command.buffer = nullptr;

    App::Device.destroyCommandPool(m_Command.pool);
    m_Command.pool = nullptr;

    return true;
}

bool Transfer::map(const Buffer &buffer, const std::span<const std::byte> &bytes, std::uint64_t offset) {
    // prepare a one time command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));
    PBZ_VK_CHECK_RESULT(m_Command.buffer.reset());

    PBZ_VK_CHECK_RESULT(m_Command.buffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    {
        vk::BufferMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eNone,
            .srcAccessMask = vk::AccessFlagBits2::eNone,
            .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
            .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .buffer = buffer.getData().buffer,
            .offset = offset,
            .size = bytes.size(),
        };

        m_Command.buffer.pipelineBarrier2({
            .dependencyFlags = {},
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &barrier,
        });
    }

    if (!buffer.map(m_Command.buffer, &m_Deletion, bytes, offset)) {
        Logger::ERROR("[Transfer] Failed to map buffer.");
    }

    PBZ_VK_CHECK_RESULT(m_Command.buffer.end());

    // submit
    const vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffer,
    };

    PBZ_VK_CHECK_RESULT(App::Queues.transfer.submit(submitInfo, m_Fences.submit));

    // release the staging buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    m_Deletion.flush();

    return true;
}

bool Transfer::map(const Image &image, const std::span<const std::byte> &bytes, vk::ImageLayout layout) {
    // prepare the command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));
    PBZ_VK_CHECK_RESULT(m_Command.buffer.reset());

    PBZ_VK_CHECK_RESULT(m_Command.buffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eNone,
            .srcAccessMask = vk::AccessFlagBits2::eNone,
            .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
            .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eTransferDstOptimal,
            .image = image.getData().image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = image.getInfo().mipLevels,
                .baseArrayLayer = 0,
                .layerCount = image.getInfo().arrayLayers,
            },
        };

        m_Command.buffer.pipelineBarrier2({
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        });
    }

    if (!image.map(m_Command.buffer, &m_Deletion, bytes)) {
        Logger::ERROR("[Transfer] Failed to map image.");
    }

    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
            .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eNone,
            .dstAccessMask = vk::AccessFlagBits2::eNone,
            .oldLayout = vk::ImageLayout::eTransferDstOptimal,
            .newLayout = layout,
            .image = image.getData().image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = image.getInfo().mipLevels,
                .baseArrayLayer = 0,
                .layerCount = image.getInfo().arrayLayers,
            },
        };

        m_Command.buffer.pipelineBarrier2({
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        });
    }

    // submit
    PBZ_VK_CHECK_RESULT(m_Command.buffer.end());

    const vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffer,
    };

    PBZ_VK_CHECK_RESULT(App::Queues.transfer.submit(submitInfo, m_Fences.submit));

    // release the staging buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    m_Deletion.flush();

    return true;
}

void Transfer::immediate(std::function<void(vk::CommandBuffer)> record) {
    // prepare the command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));
    PBZ_VK_CHECK_RESULT(m_Command.buffer.reset());

    PBZ_VK_CHECK_RESULT(m_Command.buffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    record(m_Command.buffer);

    PBZ_VK_CHECK_RESULT(m_Command.buffer.end());

    // submit
    vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffer,
    };

    PBZ_VK_CHECK_RESULT(App::Queues.transfer.submit(submitInfo, m_Fences.submit));
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
}

} // namespace Physbuzz
