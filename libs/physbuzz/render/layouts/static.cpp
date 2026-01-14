#include "static.hpp"

#include "../../app/application.hpp"
#include "../renderers/defines.hpp"

namespace Physbuzz {

StaticBuffer::StaticBuffer()
    : m_Buffer({
          .usage = Buffer::BufferUsageFlagBits::eTransferSrc | Buffer::BufferUsageFlagBits::eTransferDst | Buffer::BufferUsageFlagBits::eShaderDeviceAddress,
          .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
      }) {}

bool StaticBuffer::build(std::uint64_t size) {
    if (m_Address != 0) {
        Logger::WARNING("[StaticBuffer] Trying to build a constructed static buffer.");
        return true;
    }

    m_Buffer = {{
        .usage = Buffer::BufferUsageFlagBits::eTransferSrc | Buffer::BufferUsageFlagBits::eTransferDst | Buffer::BufferUsageFlagBits::eShaderDeviceAddress,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};

    if (!m_Buffer.build(size)) {
        return false;
    }

    m_Address = App::Device.getBufferAddress({
        .buffer = m_Buffer.getData().buffer,
    });

    return true;
}

bool StaticBuffer::destroy() {
    if (m_Address == 0) {
        Logger::WARNING("[StaticBuffer] Trying to destroy a destructed static buffer.");
        return true;
    }

    if (!m_Buffer.destroy()) {
        return false;
    }

    m_Address = 0;
    return true;
}

bool StaticBuffer::resize(const RenderContext &context, const std::shared_ptr<Transfer> transfer, std::uint64_t size) {
    if (m_Buffer.getData().bufferInfo.size >= size || size == 0) {
        return false;
    }

    Buffer buffer = m_Buffer.getInfo();

    // create new buffer
    if (!buffer.build(size)) {
        Logger::ERROR("[StaticBuffer] Failed to create new shader buffers.");
        return false;
    }

    // copy old data to the new buffer
    std::vector<vk::BufferCopy> copies = {{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = glm::min(m_Buffer.getData().bufferInfo.size, buffer.getData().bufferInfo.size),
    }};

    transfer->immediate([&](const vk::CommandBuffer &command) {
        buffer.copy(command, m_Buffer, copies);
    });

    // mark old buffer for deferred deletion and update
    context.deletionQueue->enqueue(m_Buffer);

    m_Buffer = buffer;
    m_Address = App::Device.getBufferAddress({
        .buffer = m_Buffer.getData().buffer,
    });

    return true;
}

bool StaticBuffer::update(const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const {
    PBZ_ASSERT(m_Address != 0, "[StaticBuffer] Buffer has not been allocated.");
    PBZ_ASSERT(offset + bytes.size() <= m_Buffer.getData().bufferInfo.size, "[StaticBuffer] Invalid size and offset.");

    return transfer->map(m_Buffer, bytes, offset);
}

const Buffer &StaticBuffer::getBuffer() const {
    return m_Buffer;
}

const vk::DeviceAddress &StaticBuffer::getAddress() const {
    return m_Address;
}

} // namespace Physbuzz
