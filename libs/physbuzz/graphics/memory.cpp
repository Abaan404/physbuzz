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
        return true;
    }

    // otherwise stage the buffer and copy from host to vram
    Buffer stagingBuffer = {{
        .usage = Buffer::UsageFlagBits::eTransferSrc,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};

    if (!stagingBuffer.build(bytes.size())) {
        Logger::ERROR("[Transfer] Failed to build staging buffer.");
        return false;
    }

    PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCopyMemoryToAllocation(App::Allocator, bytes.data(), stagingBuffer.m_Allocation, offset, bytes.size())));
    vmaFlushAllocation(App::Allocator, stagingBuffer.m_Allocation, offset, bytes.size());

    vk::BufferCopy copy = {
        .srcOffset = 0,
        .dstOffset = offset,
        .size = bytes.size(),
    };

    vk::BufferMemoryBarrier2 barrier = {
        .srcStageMask = vk::PipelineStageFlagBits2::eHost,
        .srcAccessMask = vk::AccessFlagBits2::eHostWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
        .dstAccessMask = vk::AccessFlagBits2::eTransferRead,
        .buffer = stagingBuffer.m_Data.buffer,
        .offset = 0,
        .size = bytes.size(),
    };

    cmd.pipelineBarrier2({
        .dependencyFlags = {},
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &barrier,
    });

    cmd.copyBuffer(stagingBuffer.m_Data.buffer, m_Data.buffer, copy);

    deletion->enqueue(std::move(stagingBuffer));

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

bool Image::map(vk::CommandBuffer cmd, DeletionQueue *deletion, const std::span<const std::byte> &bytes) const {
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

    vk::BufferImageCopy copy = {
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

    {
        vk::BufferMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eHost,
            .srcAccessMask = vk::AccessFlagBits2::eHostWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
            .dstAccessMask = vk::AccessFlagBits2::eTransferRead,
            .buffer = stagingBuffer.m_Data.buffer,
            .offset = 0,
            .size = bytes.size(),
        };

        cmd.pipelineBarrier2({
            .dependencyFlags = {},
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &barrier,
        });
    }

    cmd.copyBufferToImage(stagingBuffer.m_Data.buffer, m_Data.image, vk::ImageLayout::eTransferDstOptimal, {copy});

    // release the staging buffer
    deletion->enqueue(std::move(stagingBuffer));

    return true;
}

const Image::Info &Image::getInfo() const {
    return m_Info;
}

const Image::Data &Image::getData() const {
    return m_Data;
}

} // namespace Physbuzz
