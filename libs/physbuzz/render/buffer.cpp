#include "buffer.hpp"

namespace Physbuzz {

Buffer::Buffer(const Info &info)
    : m_Info(info) {}

bool Buffer::build(std::uint64_t size) {
    if (m_Data.buffer != nullptr) {
        Logger::WARNING("[Buffer] Trying to build a constructed buffer.");
        return true;
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

bool Buffer::copy(const vk::CommandBuffer &commandBuffer, const Buffer &src, std::vector<vk::BufferCopy> copies) const {
    if (m_Data.buffer == nullptr || m_Allocation == nullptr) {
        Logger::ERROR("[Buffer] Trying to copy to a destructed buffer.");
        return false;
    }

    if (src.m_Data.buffer == nullptr || src.m_Allocation == nullptr) {
        Logger::ERROR("[Buffer] Trying to copy from a destructed buffer.");
        return false;
    }

    for (const auto &copy : copies) {
        PBZ_ASSERT(copy.srcOffset + copy.size <= src.m_Data.bufferInfo.size, "[Buffer] Not enough space in source buffer to copy from.");
        PBZ_ASSERT(copy.dstOffset + copy.size <= m_Data.bufferInfo.size, "[Buffer] Not enough space in destination buffer to copy to.");
    }

    commandBuffer.copyBuffer(src.m_Data.buffer, m_Data.buffer, copies);

    return true;
}

bool Buffer::map(const std::span<const std::byte> &data, std::uint64_t offset) const {
    PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCopyMemoryToAllocation(App::Allocator, data.data(), m_Allocation, offset, data.size())));

    VkMemoryPropertyFlags memProps = 0;
    vmaGetAllocationMemoryProperties(App::Allocator, m_Allocation, &memProps);
    if (!(memProps & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        vmaFlushAllocation(App::Allocator, m_Allocation, offset, data.size());
    }

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

bool Image::copy(const vk::CommandBuffer &commandBuffer, const Buffer &src) const {
    if (m_Data.image == nullptr || m_Allocation == nullptr) {
        Logger::ERROR("[Image] Trying to copy to a destructed image.");
        return false;
    }

    if (src.m_Data.buffer == nullptr || src.m_Allocation == nullptr) {
        Logger::ERROR("[Image] Trying to copy from a destructed image.");
        return false;
    }

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

    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
                .srcAccessMask = {},
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

        commandBuffer.pipelineBarrier2({
            .dependencyFlags = {},
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    commandBuffer.copyBufferToImage(src.getData().buffer, m_Data.image, vk::ImageLayout::eTransferDstOptimal, {region});

    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                .dstAccessMask = vk::AccessFlagBits2::eTransferRead,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
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

        commandBuffer.pipelineBarrier2({
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
    if (buffer.m_Allocation == nullptr) {
        Logger::ERROR("[Transfer] Cannot Transfer an uninitialized allocation.");
        return false;
    }

    // prepare a one time command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));
    m_Command.buffer.reset();

    PBZ_VK_CHECK_RESULT(m_Command.buffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(App::Allocator, buffer.m_Allocation, &memPropFlags);

    Buffer stagingBuffer = {{
        .usage = Buffer::BufferUsageFlagBits::eTransferSrc,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};

    // if the buffer can be read by the CPU (i.e. integrated gpus)
    if (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        if (!buffer.map(bytes, offset)) {
            Logger::ERROR("[Transfer] Failed to map buffer.");
            PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
            return false;
        }
    }

    // otherwise stage the buffer and copy from host to vram
    else {
        if (!stagingBuffer.build(bytes.size())) {
            Logger::ERROR("[Transfer] Failed to build staging buffer.");
            PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
            return false;
        }

        if (!stagingBuffer.map(bytes, 0)) {
            Logger::ERROR("[Transfer] Failed to map staging buffer.");
            stagingBuffer.destroy();
            PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
            return false;
        }

        std::vector<vk::BufferCopy> copies = {{
            .srcOffset = 0,
            .dstOffset = offset,
            .size = bytes.size(),
        }};

        if (!buffer.copy(m_Command.buffer, stagingBuffer, copies)) {
            Logger::ERROR("[Transfer] Failed to copy from staging buffer.");
            stagingBuffer.destroy();
            PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
            return false;
        }
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
    if (!(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        stagingBuffer.destroy();
    }

    return true;
}

bool Transfer::map(const Image &image, const std::span<const std::byte> &data) {
    if (image.m_Allocation == nullptr) {
        Logger::ERROR("[Transfer] Cannot Transfer an uninitialized allocation.");
        return false;
    }

    // prepare the command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));
    m_Command.buffer.reset();

    PBZ_VK_CHECK_RESULT(m_Command.buffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    // create a staging buffer in host memory
    Buffer stagingBuffer = {{
        .usage = Buffer::BufferUsageFlagBits::eTransferSrc,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};

    if (!stagingBuffer.build(data.size())) {
        Logger::ERROR("[Transfer] Failed to build image staging buffer.");
        PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
        return false;
    }

    if (!stagingBuffer.map(data, 0)) {
        Logger::ERROR("[Transfer] Failed to map image from staging buffer.");
        stagingBuffer.destroy();
        PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
        return false;
    }

    // copy the buffer into a VkImage on the vram
    if (!image.copy(m_Command.buffer, stagingBuffer)) {
        Logger::ERROR("[Transfer] Failed to copy image from staging buffer.");
        stagingBuffer.destroy();
        PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
        return false;
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
    stagingBuffer.destroy();

    return true;
}

void Transfer::immediate(std::function<void(const vk::CommandBuffer &)> record) {
    // prepare the command buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));
    m_Command.buffer.reset();

    PBZ_VK_CHECK_RESULT(m_Command.buffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    record(m_Command.buffer);

    PBZ_VK_CHECK_RESULT(m_Command.buffer.end());

    vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffer,
    };

    PBZ_VK_CHECK_RESULT(App::Queues.transfer.submit(submitInfo, m_Fences.submit));
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
}

} // namespace Physbuzz
