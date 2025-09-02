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

    if (m_Info.bufferUsage & vk::BufferUsageFlagBits::eVertexBuffer) {
        m_Data.accessMask |= vk::AccessFlagBits::eVertexAttributeRead;
        m_Data.stageMask |= vk::PipelineStageFlagBits::eVertexInput;
    }

    if (m_Info.bufferUsage & vk::BufferUsageFlagBits::eIndexBuffer) {
        m_Data.accessMask |= vk::AccessFlagBits::eIndexRead;
        m_Data.stageMask |= vk::PipelineStageFlagBits::eVertexInput;
    }

    if (m_Info.bufferUsage & vk::BufferUsageFlagBits::eUniformBuffer) {
        m_Data.accessMask |= vk::AccessFlagBits::eUniformRead;
        m_Data.stageMask |= vk::PipelineStageFlagBits::eVertexShader |
                            vk::PipelineStageFlagBits::eFragmentShader;
    }

    if (m_Info.bufferUsage & vk::BufferUsageFlagBits::eStorageBuffer) {
        m_Data.accessMask |= vk::AccessFlagBits::eShaderRead |
                             vk::AccessFlagBits::eShaderWrite;
        m_Data.stageMask |= vk::PipelineStageFlagBits::eComputeShader |
                            vk::PipelineStageFlagBits::eVertexShader |
                            vk::PipelineStageFlagBits::eFragmentShader;
    }

    if (m_Info.bufferUsage & vk::BufferUsageFlagBits::eTransferSrc) {
        m_Data.accessMask |= vk::AccessFlagBits::eTransferRead;
        m_Data.stageMask |= vk::PipelineStageFlagBits::eTransfer;
    }

    if (m_Info.bufferUsage & vk::BufferUsageFlagBits::eTransferDst) {
        m_Data.accessMask |= vk::AccessFlagBits::eTransferWrite;
        m_Data.stageMask |= vk::PipelineStageFlagBits::eTransfer;
    }

    if (!m_Data.accessMask) {
        Logger::ERROR("[Buffer] No access flag for buffer usage. ({})", vk::to_string(m_Info.bufferUsage));
    }

    if (!m_Data.stageMask) {
        m_Data.stageMask = vk::PipelineStageFlagBits::eTransfer;
    }

    m_Data.bufferInfo = {
        .size = size,
        .usage = m_Info.bufferUsage,
        .sharingMode = m_Info.sharingMode,
    };

    VkBufferCreateInfo cBufferInfo = static_cast<VkBufferCreateInfo>(m_Data.bufferInfo);
    VkBuffer cBuffer = static_cast<VkBuffer>(m_Data.buffer);

    VkResult res = vmaCreateBuffer(App::Allocator, &cBufferInfo, &allocInfo, &cBuffer, &m_Data.allocation, nullptr);
    if (res != VK_SUCCESS) {
        Logger::ERROR("[Buffer] vmaCreateBuffer failed. ({})", static_cast<int>(res));
        return false;
    }

    m_Data.buffer = cBuffer;

    return true;
}

bool Buffer::destroy() {
    if (m_Data.buffer == nullptr) {
        Logger::WARNING("[Buffer] Trying to destroy a destructed buffer.");
        return true;
    }

    vmaDestroyBuffer(App::Allocator, static_cast<VkBuffer>(m_Data.buffer), m_Data.allocation);
    m_Data = {};
    return true;
}

const Buffer::Info &Buffer::getInfo() const {
    return m_Info;
}

const Buffer::Data &Buffer::getData() const {
    return m_Data;
}

bool Buffer::mapBytes(const vk::CommandBuffer &commandBuffer, const std::span<const std::byte> &data) const {
    vk::Result result = static_cast<vk::Result>(vmaCopyMemoryToAllocation(App::Allocator, data.data(), m_Data.allocation, 0, data.size()));

    if (result != vk::Result::eSuccess) {
        Logger::ERROR("[Transfer] Memory transfer failed. ({})", vk::to_string(result));
        return false;
    }

    vk::BufferMemoryBarrier barrier = {
        .srcAccessMask = vk::AccessFlagBits::eHostWrite,
        .dstAccessMask = m_Data.accessMask,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .buffer = m_Data.buffer,
        .offset = 0,
        .size = vk::WholeSize,
    };

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, m_Data.stageMask, {}, nullptr, barrier, nullptr);

    return true;
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
    App::Device.destroyCommandPool(m_Command.pool);

    return true;
}

bool Transfer::mapBytes(const Buffer &buffer, const std::span<const std::byte> &data) {
    const Buffer::Data &bufferData = buffer.getData();

    if (bufferData.buffer == nullptr || bufferData.allocation == nullptr) {
        Logger::ERROR("[Transfer] Cannot Transfer an uninitialized buffer.");
        return false;
    }

    {
        vk::Result result = App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max());

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Transfer] Transfer fence failed. ({})", vk::to_string(result));
            return false;
        }
    }

    App::Device.resetFences(m_Fences.submit);
    m_Command.buffer.reset();

    {
        vk::Result result = m_Command.buffer.begin({
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        });

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Transfer] Transfer begin failed ({})", vk::to_string(result));
            return false;
        }
    }

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(App::Allocator, bufferData.allocation, &memPropFlags);

    if (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        if (!buffer.mapBytes(m_Command.buffer, data)) {
            return false;
        }

    } else {
        Buffer stagingBuffer = {{
            .bufferUsage = Buffer::BufferUsageFlagBits::eTransferSrc,
            .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
        }};

        if (!stagingBuffer.build(data.size())) {
            Logger::ERROR("[Transfer] Failed to build a transfer buffer.");
            return false;
        }

        if (!stagingBuffer.mapBytes(m_Command.buffer, data)) {
            stagingBuffer.destroy();
            return false;
        }

        vk::BufferCopy copy = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = data.size(),
        };

        m_Command.buffer.copyBuffer(stagingBuffer.getData().buffer, bufferData.buffer, 1, &copy);

        vk::BufferMemoryBarrier dstBarrier = {
            .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
            .dstAccessMask = bufferData.accessMask,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .buffer = bufferData.buffer,
            .offset = 0,
            .size = vk::WholeSize,
        };

        m_Command.buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, bufferData.stageMask, {}, nullptr, dstBarrier, nullptr);
    }

    {
        vk::Result result = m_Command.buffer.end();

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Transfer] Transfer end failed. ({})", vk::to_string(result));
            return false;
        }
    }

    {
        const vk::SubmitInfo submitInfo = {
            .commandBufferCount = 1,
            .pCommandBuffers = &m_Command.buffer,
        };
        vk::Result result = App::Queues.transfer.submit(submitInfo, m_Fences.submit);

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Queue submission failed. ({})", vk::to_string(result));
            return false;
        }
    }

    return true;
}

} // namespace Physbuzz
