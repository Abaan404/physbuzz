#include "memory.hpp"

#include "../app/application.hpp"
#include "../app/deletion.hpp"

namespace Physbuzz {

Buffer::Buffer(const Info &info)
    : m_Info(info) {}

bool Buffer::build(std::uint64_t size) {
    if (m_Data.buffer != nullptr) {
        Logger::WARNING("[Buffer] Trying to build a constructed buffer.");
        return true;
    }

    if (size == 0) {
        Logger::ERROR("[Buffer] Trying to build an empty buffer.");
        return false;
    }

    VmaAllocationCreateInfo allocInfo = {};

    switch (m_Info.memoryUsage) {
    case MemoryUsage::Auto:
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        break;

    case MemoryUsage::CPUOnly:
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                          VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;

    case Buffer::MemoryUsage::GPUOnly:
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        break;

    case Buffer::MemoryUsage::CPUToGPU:
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                          VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;

    case Buffer::MemoryUsage::GPUToCPU:
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        break;
    }

    m_Data.bufferInfo = {
        .flags = m_Info.flags,
        .size = size,
        .usage = m_Info.usage,
        .sharingMode = m_Info.sharingMode,
    };

    VkBufferCreateInfo bufferInfo = static_cast<VkBufferCreateInfo>(m_Data.bufferInfo);
    VkBuffer buffer = static_cast<VkBuffer>(m_Data.buffer);
    PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCreateBuffer(App::Allocator, &bufferInfo, &allocInfo, &buffer, &m_Allocation, nullptr)));
    m_Data.buffer = buffer;

    return true;
}

bool Buffer::destroy() {
    if (m_Data.buffer == nullptr) {
        Logger::WARNING("[Buffer] Trying to destroy a destructed buffer.");
        return true;
    }

    vmaDestroyBuffer(App::Allocator, static_cast<VkBuffer>(m_Data.buffer), m_Allocation);
    m_Data.buffer = nullptr;
    m_Allocation = nullptr;

    return true;
}

const Buffer::Info &Buffer::getInfo() const {
    return m_Info;
}

const Buffer::Data &Buffer::getData() const {
    return m_Data;
}

bool Buffer::map(vk::CommandBuffer cmd, DeletionQueue *deletion, const std::span<const std::byte> &bytes, std::uint64_t offset) const {
    if (m_Allocation == nullptr) {
        Logger::ERROR("[Transfer] Cannot Transfer an uninitialized allocation.");
        return false;
    }

    VkMemoryPropertyFlags memProps = 0;
    vmaGetAllocationMemoryProperties(App::Allocator, m_Allocation, &memProps);

    // if the buffer can be read by the CPU (i.e. integrated gpus)
    if (memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCopyMemoryToAllocation(App::Allocator, bytes.data(), m_Allocation, offset, bytes.size())));
    }

    // otherwise stage the buffer and copy from host to vram
    else {
        Buffer stagingBuffer = {{
            .usage = Buffer::UsageFlagBits::eTransferSrc,
            .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
        }};

        if (!stagingBuffer.build(bytes.size())) {
            Logger::ERROR("[Transfer] Failed to build staging buffer.");
            return false;
        }

        PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCopyMemoryToAllocation(App::Allocator, bytes.data(), stagingBuffer.m_Allocation, offset, bytes.size())));
        vmaFlushAllocation(App::Allocator, m_Allocation, offset, bytes.size());

        std::vector<vk::BufferCopy> copies = {{
            .srcOffset = 0,
            .dstOffset = offset,
            .size = bytes.size(),
        }};

        if (!copy(cmd, stagingBuffer, copies)) {
            Logger::ERROR("[Transfer] Failed to copy from staging buffer.");
            stagingBuffer.destroy();
            return false;
        }

        deletion->enqueue(std::move(stagingBuffer));
    }

    return true;
}

bool Buffer::copy(vk::CommandBuffer cmd, const Buffer &src, const std::vector<vk::BufferCopy> &copies) const {
    if (m_Data.buffer == nullptr || m_Allocation == nullptr) {
        Logger::ERROR("[Buffer] Trying to copy to a destructed buffer.");
        return false;
    }

    if (src.m_Data.buffer == nullptr || src.m_Allocation == nullptr) {
        Logger::ERROR("[Buffer] Trying to copy from a destructed buffer.");
        return false;
    }

    std::vector<vk::BufferMemoryBarrier2> preCopyBarriers;
    std::vector<vk::BufferMemoryBarrier2> postCopyBarriers;

    preCopyBarriers.reserve(copies.size());
    postCopyBarriers.reserve(copies.size());

    for (const auto &copy : copies) {
        PBZ_ASSERT(copy.srcOffset + copy.size <= src.m_Data.bufferInfo.size, "[Buffer] Not enough space in source buffer to copy from.");
        PBZ_ASSERT(copy.dstOffset + copy.size <= m_Data.bufferInfo.size, "[Buffer] Not enough space in destination buffer to copy to.");

        preCopyBarriers.emplace_back<vk::BufferMemoryBarrier2>({
            .srcStageMask = vk::PipelineStageFlagBits2::eNone,
            .srcAccessMask = vk::AccessFlagBits2::eNone,
            .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
            .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .buffer = m_Data.buffer,
            .offset = copy.dstOffset,
            .size = copy.size,
        });

        postCopyBarriers.emplace_back<vk::BufferMemoryBarrier2>({
            .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
            .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
            .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
            .buffer = m_Data.buffer,
            .offset = copy.dstOffset,
            .size = copy.size,
        });
    }

    cmd.pipelineBarrier2({
        .dependencyFlags = {},
        .bufferMemoryBarrierCount = static_cast<std::uint32_t>(preCopyBarriers.size()),
        .pBufferMemoryBarriers = preCopyBarriers.data(),
    });

    cmd.copyBuffer(src.m_Data.buffer, m_Data.buffer, copies);

    cmd.pipelineBarrier2({
        .dependencyFlags = {},
        .bufferMemoryBarrierCount = static_cast<std::uint32_t>(postCopyBarriers.size()),
        .pBufferMemoryBarriers = postCopyBarriers.data(),
    });

    return true;
}

Image::Image(const Info &info)
    : m_Info(info) {}

bool Image::build(const glm::uvec3 &extent) {
    m_Data.imageInfo = {
        .flags = m_Info.flags,
        .imageType = m_Info.type,
        .format = m_Info.format,
        .extent = {extent.x, extent.y, extent.z},
        .mipLevels = m_Info.mipLevels,
        .arrayLayers = m_Info.arrayLayers,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = m_Info.usage,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCreateInfo.priority = 1.0f;

    VkImage image;
    VkImageCreateInfo imageInfo = static_cast<VkImageCreateInfo>(m_Data.imageInfo);
    PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCreateImage(App::Allocator, &imageInfo, &allocCreateInfo, &image, &m_Allocation, nullptr)));
    m_Data.image = image;

    return true;
}

bool Image::destroy() {
    if (m_Data.image == nullptr) {
        Logger::WARNING("[Image] Trying to destroy a destructed image.");
        return true;
    }

    vmaDestroyImage(App::Allocator, static_cast<VkImage>(m_Data.image), m_Allocation);
    m_Data.image = nullptr;
    m_Allocation = nullptr;

    return true;
}

bool Image::map(vk::CommandBuffer cmd, DeletionQueue *deletion, const std::span<const std::byte> &bytes, vk::ImageLayout layout) const {
    if (m_Allocation == nullptr) {
        Logger::ERROR("[Transfer] Cannot Transfer an uninitialized allocation.");
        return false;
    }

    // create a staging buffer in host memory
    Buffer stagingBuffer = {{
        .usage = Buffer::UsageFlagBits::eTransferSrc,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};

    if (!stagingBuffer.build(bytes.size())) {
        Logger::ERROR("[Transfer] Failed to build image staging buffer.");
        return false;
    }

    PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCopyMemoryToAllocation(App::Allocator, bytes.data(), stagingBuffer.m_Allocation, 0, bytes.size())));
    vmaFlushAllocation(App::Allocator, stagingBuffer.m_Allocation, 0, bytes.size());

    vk::BufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = m_Info.arrayLayers,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = m_Data.imageInfo.extent,
    };

    // copy the buffer into a VkImage on the vram
    if (!copy(cmd, stagingBuffer, {region}, layout)) {
        Logger::ERROR("[Transfer] Failed to copy image from staging buffer.");
        stagingBuffer.destroy();
        return false;
    }

    // release the staging buffer
    deletion->enqueue(std::move(stagingBuffer));

    return true;
}

bool Image::copy(vk::CommandBuffer cmd, const Buffer &src, const std::vector<vk::BufferImageCopy> &copies, vk::ImageLayout layout) const {
    if (m_Data.image == nullptr || m_Allocation == nullptr) {
        Logger::ERROR("[Image] Trying to copy to a destructed image.");
        return false;
    }

    if (src.m_Data.buffer == nullptr || src.m_Allocation == nullptr) {
        Logger::ERROR("[Image] Trying to copy from a destructed buffer.");
        return false;
    }

    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
                .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .image = m_Data.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = m_Info.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = m_Info.arrayLayers,
                },
            },
        };

        cmd.pipelineBarrier2({
            .dependencyFlags = {},
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    cmd.copyBufferToImage(src.getData().buffer, m_Data.image, vk::ImageLayout::eTransferDstOptimal, copies);

    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                .dstAccessMask = vk::AccessFlagBits2::eTransferRead,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = layout,
                .image = m_Data.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = m_Info.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = m_Info.arrayLayers,
                },
            },
        };

        cmd.pipelineBarrier2({
            .dependencyFlags = {},
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    return true;
}

bool Image::copy(vk::CommandBuffer cmd, const Image &src, const std::vector<vk::ImageCopy> &copies, vk::ImageLayout layout) const {
    if (m_Data.image == nullptr || m_Allocation == nullptr) {
        Logger::ERROR("[Image] Trying to copy to a destructed image.");
        return false;
    }

    if (src.m_Data.image == nullptr || src.m_Allocation == nullptr) {
        Logger::ERROR("[Image] Trying to copy from a destructed image.");
        return false;
    }

    {
        // Note: eUndefined trashes the old image data/layout, this could bite me later.
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
                .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .image = m_Data.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = m_Info.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = m_Info.arrayLayers,
                },
            },
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
                .dstAccessMask = vk::AccessFlagBits2::eTransferRead,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferSrcOptimal,
                .image = src.m_Data.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = src.m_Info.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = src.m_Info.arrayLayers,
                },
            },
        };

        cmd.pipelineBarrier2({
            .dependencyFlags = {},
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    cmd.copyImage(src.getData().image, vk::ImageLayout::eTransferSrcOptimal, m_Data.image, vk::ImageLayout::eTransferDstOptimal, {copies});

    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                .dstAccessMask = vk::AccessFlagBits2::eTransferRead,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = layout,
                .image = m_Data.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = m_Info.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = m_Info.arrayLayers,
                },
            },
        };

        cmd.pipelineBarrier2({
            .dependencyFlags = {},
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    return true;
}

const Image::Info &Image::getInfo() const {
    return m_Info;
}

const Image::Data &Image::getData() const {
    return m_Data;
}

} // namespace Physbuzz
