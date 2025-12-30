#include "static.hpp"

namespace Physbuzz {

bool StaticBuffer::build() {
    if (!m_Buffers.empty()) {
        Logger::WARNING("[StaticBuffer] Trying to build a constructed static buffer.");
        return true;
    }

    Buffer::BufferUsageFlagBits usage;
    switch (m_Type) {
    case Type::Constant:
        usage = Buffer::BufferUsageFlagBits::eUniformBuffer;
        break;

    case Type::Structured:
        usage = Buffer::BufferUsageFlagBits::eStorageBuffer;
        break;
    }

    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        m_Buffers.emplace_back(Buffer::Info{
            .usage = usage | Buffer::BufferUsageFlagBits::eTransferDst,
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

const std::vector<Buffer> &StaticBuffer::getBuffers() const {
    return m_Buffers;
}

std::size_t StaticBuffer::getRange() const {
    return m_Stride * m_Count;
}

StaticBuffer::Type StaticBuffer::getType() const {
    return m_Type;
}

} // namespace Physbuzz
