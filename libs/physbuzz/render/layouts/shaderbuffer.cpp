#include "shaderbuffer.hpp"

namespace Physbuzz {

ShaderBuffer::ShaderBuffer(const Info &info)
    : m_Info(info) {}

bool ShaderBuffer::build(std::uint64_t size) {
    if (!m_Buffers.empty()) {
        Logger::WARNING("[ShaderBuffer] Trying to build a constructed static buffer.");
        return true;
    }

    Buffer::BufferUsageFlagBits usage;
    switch (m_Info.type) {
    case Type::ConstantDynamic:
    case Type::Constant:
        usage = Buffer::BufferUsageFlagBits::eUniformBuffer;
        break;

    case Type::StructuredDynamic:
    case Type::Structured:
        usage = Buffer::BufferUsageFlagBits::eStorageBuffer;
        break;
    }

    for (std::size_t i = 0; i < detail::getLayoutLifetimeSetCount(m_Info.lifetime); i++) {
        m_Buffers.emplace_back<Buffer::Info>({
            .usage = usage | Buffer::BufferUsageFlagBits::eTransferSrc | Buffer::BufferUsageFlagBits::eTransferDst,
            .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
        });

        if (!m_Buffers[i].build(size)) {
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

bool ShaderBuffer::resize(RenderContext context, std::uint64_t size) {
    Buffer buffer = m_Buffers[context.frameInFlight].getInfo();

    PBZ_ASSERT(m_Info.lifetime == LayoutLifetime::PerFrame, "[ShaderBuffer] Cannot resize this buffer without a per-frame lifetime using a render context.");

    if (m_Buffers[context.frameInFlight].getData().bufferInfo.size >= size || size == 0) {
        return false;
    }

    // create new buffer
    if (!buffer.build(size)) {
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
    std::size_t index;
    switch (m_Info.lifetime) {
    case LayoutLifetime::Global:
        index = 0;
        break;
    case LayoutLifetime::PerFrame:
        index = context.frameInFlight;
        break;
    }

    PBZ_ASSERT(!m_Buffers.empty(), "[ShaderBuffer] Buffer has not been allocated.");
    PBZ_ASSERT(offset + bytes.size() <= m_Buffers[index].getData().bufferInfo.size, "[ShaderBuffer] Invalid size and offset.");

    return transfer->map(m_Buffers[index], bytes, offset);
}

const ShaderBuffer::Info &ShaderBuffer::getInfo() const {
    return m_Info;
}

const std::vector<Buffer> &ShaderBuffer::getBuffers() const {
    return m_Buffers;
}

} // namespace Physbuzz
