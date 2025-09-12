#include "uniform.hpp"

namespace Physbuzz {

const std::vector<Buffer> &Uniform::getBuffers() const {
    return m_Buffers;
}

std::size_t Uniform::getRange() const {
    return m_Stride * m_Count;
}

bool Uniform::build() {
    if (m_Buffers.size() != 0) {
        Logger::WARNING("[Uniform] Trying to build a constructed uniform.");
        return true;
    }

    m_Buffers.reserve(detail::MAX_FRAMES_IN_FLIGHT);
    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        Buffer &buffer = m_Buffers.emplace_back<Buffer>({{
            .usage = Buffer::BufferUsageFlagBits::eUniformBuffer | Buffer::BufferUsageFlagBits::eTransferDst,
            .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
        }});

        if (!buffer.build(m_Stride * m_Count)) {
            destroy();
            return false;
        }
    }

    return true;
}

bool Uniform::destroy() {
    if (m_Buffers.size() == 0) {
        Logger::WARNING("[Uniform] Trying to destroy a destructed uniform.");
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
