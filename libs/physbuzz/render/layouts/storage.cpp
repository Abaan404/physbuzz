#include "storage.hpp"

namespace Physbuzz {

const std::vector<Buffer> &StorageBuffer::getBuffers() const {
    return m_Buffers;
}

std::size_t StorageBuffer::getRange() const {
    return m_Stride * m_Count;
}

bool StorageBuffer::build() {
    if (m_Buffers.size() != 0) {
        Logger::WARNING("[StorageBuffer] Trying to build a constructed uniform.");
        return true;
    }

    m_Buffers.reserve(detail::MAX_FRAMES_IN_FLIGHT);
    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        Buffer &buffer = m_Buffers.emplace_back<Buffer>({{
            .usage = Buffer::BufferUsageFlagBits::eStorageBuffer | Buffer::BufferUsageFlagBits::eTransferDst,
            .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
        }});

        if (!buffer.build(m_Stride * m_Count)) {
            destroy();
            return false;
        }
    }

    return true;
}

bool StorageBuffer::destroy() {
    if (m_Buffers.size() == 0) {
        Logger::WARNING("[StorageBuffer] Trying to destroy a destructed uniform.");
        return true;
    }

    bool success = true;

    for (auto &buffer : m_Buffers) {
        success &= buffer.destroy();
    }

    m_Buffers.clear();
    return success;
}

} // namespace Physbuzz
