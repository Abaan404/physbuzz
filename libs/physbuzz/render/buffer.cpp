#include "buffer.hpp"

namespace Physbuzz {

Buffer::Buffer(const Info &info)
    : m_Info(info) {}

bool Buffer::build(std::size_t size) {
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

void Buffer::copy(const vk::CommandBuffer &commandBuffer, const Buffer &srcBuffer) const {
    PBZ_ASSERT(srcBuffer.m_Data.bufferInfo.size <= m_Data.bufferInfo.size, "[Buffer] Dst buffer size too small.");

    vk::BufferCopy copy = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = srcBuffer.m_Data.bufferInfo.size,
    };

    commandBuffer.copyBuffer(srcBuffer.m_Data.buffer, m_Data.buffer, 1, &copy);
}

bool Buffer::map(const std::span<const std::byte> &data) const {
    PBZ_VK_CHECK_RESULT(static_cast<vk::Result>(vmaCopyMemoryToAllocation(App::Allocator, data.data(), m_Allocation, 0, data.size())));

    VkMemoryPropertyFlags memProps = 0;
    vmaGetAllocationMemoryProperties(App::Allocator, m_Allocation, &memProps);
    if (!(memProps & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        vmaFlushAllocation(App::Allocator, m_Allocation, 0, data.size());
    }

    return true;
}

Image::Image(const Info &info)
    : m_Info(info) {}

bool Image::build(const glm::uvec3 &extent) {
    m_Data.imageInfo = {
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

    // imgCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

void Image::copy(const vk::CommandBuffer &commandBuffer, const Buffer &srcBuffer) const {
    PBZ_ASSERT(srcBuffer.getData().bufferInfo.size <= (m_Data.imageInfo.extent.width * m_Data.imageInfo.extent.height * m_Data.imageInfo.extent.depth), "[Image] Destination Image size too small.");

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
        vk::ImageMemoryBarrier barrier = {
            .srcAccessMask = {},
            .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
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
        };

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, nullptr, barrier);
    }

    commandBuffer.copyBufferToImage(srcBuffer.getData().buffer, m_Data.image, vk::ImageLayout::eTransferDstOptimal, {region});

    {
        vk::ImageMemoryBarrier barrier = {
            .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
            .dstAccessMask = vk::AccessFlagBits::eShaderRead,
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
        };

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, nullptr, barrier);
    }
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

bool Transfer::map(const Buffer &buffer, const std::span<const std::byte> &bytes) {
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
        if (!buffer.map(bytes)) {
            Logger::ERROR("[Transfer] Failed to map buffer.");
            PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
            return false;
        }
    }

    // otherwise stage the buffer and copy from host to vram
    else {
        if (!stagingBuffer.build(bytes.size())) {
            Logger::ERROR("[Transfer] Failed to build a transfer buffer.");
            PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
            return false;
        }

        if (!stagingBuffer.map(bytes)) {
            Logger::ERROR("[Transfer] Failed to map staging buffer.");
            stagingBuffer.destroy();
            PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
            return false;
        }

        buffer.copy(m_Command.buffer, stagingBuffer);
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
        Logger::ERROR("[Transfer] Failed to build a transfer buffer.");
        PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
        return false;
    }

    if (!stagingBuffer.map(data)) {
        Logger::ERROR("[Transfer] Failed to map staging buffer.");
        stagingBuffer.destroy();
        PBZ_VK_CHECK_RESULT(m_Command.buffer.end());
        return false;
    }

    // copy the buffer into a VkImage on the vram
    image.copy(m_Command.buffer, stagingBuffer);

    // submit
    PBZ_VK_CHECK_RESULT(m_Command.buffer.end());

    const vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffer,
    };

    PBZ_VK_CHECK_RESULT(App::Queues.transfer.submit(submitInfo, m_Fences.submit));
    PBZ_VK_CHECK_RESULT(App::Device.waitIdle());

    // release the staging buffer
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
    stagingBuffer.destroy();

    return true;
}

} // namespace Physbuzz
