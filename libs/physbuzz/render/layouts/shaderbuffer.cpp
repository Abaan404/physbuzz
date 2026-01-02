#include "shaderbuffer.hpp"

namespace Physbuzz {

bool ShaderBuffer::build(std::uint64_t count) {
    if (!m_Buffers.empty()) {
        Logger::WARNING("[ShaderBuffer] Trying to build a constructed static buffer.");
        return true;
    }

    Buffer::BufferUsageFlagBits usage;
    switch (m_Type) {
    case Type::ConstantDynamic:
    case Type::Constant:
        usage = Buffer::BufferUsageFlagBits::eUniformBuffer;
        break;

    case Type::StructuredDynamic:
    case Type::Structured:
        usage = Buffer::BufferUsageFlagBits::eStorageBuffer;
        break;
    }

    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        m_Buffers.emplace_back(Buffer::Info{
            .usage = usage | Buffer::BufferUsageFlagBits::eTransferSrc | Buffer::BufferUsageFlagBits::eTransferDst,
            .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
        });

        if (!m_Buffers[i].build(m_Stride * count)) {
            for (auto &buffer : m_Buffers) {
                buffer.destroy();
            }

            m_Buffers.clear();
            return false;
        }
    }

    return true;
}

bool ShaderBuffer::destroy() {
    if (m_Buffers.empty()) {
        Logger::WARNING("[ShaderBuffer] Trying to destroy a destructed static buffer.");
        return true;
    }

    bool success = true;
    for (auto &buffer : m_Buffers) {
        success &= buffer.destroy();
    }

    if (success) {
        m_Buffers.clear();
    }

    return success;
}

bool ShaderBuffer::resize(RenderContext context, std::uint64_t count) {
    Buffer buffer = m_Buffers[context.frameInFlight].getInfo();

    // create new buffer
    if (!buffer.build(m_Stride * count)) {
        Logger::ERROR("[ShaderBuffer] Failed to create new shader buffers.");
        return false;
    }

    // copy old data to the new buffer
    std::vector<vk::BufferCopy> copies = {{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = glm::min(m_Buffers[context.frameInFlight].getData().bufferInfo.size, buffer.getData().bufferInfo.size),
    }};

    buffer.copy(context.command, m_Buffers[context.frameInFlight], copies);

    // mark old buffer for deferred deletion and update
    context.deletionQueue->enqueue(m_Buffers[context.frameInFlight]);
    m_Buffers[context.frameInFlight] = buffer;

    return true;
}

bool ShaderBuffer::update(const RenderContext &context, const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const {
    PBZ_ASSERT(!m_Buffers.empty(), "[ShaderBuffer] Buffer has not been allocated.");
    PBZ_ASSERT(offset + bytes.size() <= m_Buffers[context.frameInFlight].getData().bufferInfo.size, "[ShaderBuffer] Invalid size and offset.");

    return transfer->map(m_Buffers[context.frameInFlight], bytes, offset);
}

const std::vector<Buffer> &ShaderBuffer::getBuffers() const {
    return m_Buffers;
}

std::uint64_t ShaderBuffer::getStride() const {
    return m_Stride;
}

ShaderBuffer::Type ShaderBuffer::getType() const {
    return m_Type;
}

} // namespace Physbuzz
