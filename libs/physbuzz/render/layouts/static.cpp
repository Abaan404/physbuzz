#include "static.hpp"

namespace Physbuzz {

bool StaticBuffer::build() {
    if (!m_Buffers.empty()) {
        Logger::WARNING("[StaticBuffer] Trying to build a constructed static buffer.");
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

        if (!m_Buffers[i].build(m_Stride * m_Count)) {
            destroy();
            return false;
        }
    }

    return true;
}

bool StaticBuffer::destroy() {
    if (m_Buffers.empty()) {
        Logger::WARNING("[StaticBuffer] Trying to destroy a destructed static buffer.");
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

bool StaticBuffer::update(const std::shared_ptr<Renderer> renderer, const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const {
    PBZ_ASSERT(!m_Buffers.empty(), "[StaticBuffer] Buffer has not been allocated.");
    PBZ_ASSERT(bytes.size() <= m_Stride * m_Count, "[StaticBuffer] Invalid size.");

    return transfer->map(m_Buffers[renderer->getFrameInFlight()], bytes, offset);
}

const std::vector<Buffer> &StaticBuffer::getBuffers() const {
    return m_Buffers;
}

std::uint64_t StaticBuffer::getSize() const {
    return m_Stride * m_Count;
}

std::uint64_t StaticBuffer::getStride() const {
    return m_Stride;
}

StaticBuffer::Type StaticBuffer::getType() const {
    return m_Type;
}

} // namespace Physbuzz
