#include "dynamic.hpp"

#include "../../app/deletion.hpp"
#include "../../events/descriptor.hpp"

namespace Physbuzz {

DynamicBuffer::DynamicBuffer(const Info &info)
    : m_Info(info), m_Buffers(std::monostate()) {}

bool DynamicBuffer::build(std::uint64_t size) {
    if (!std::holds_alternative<std::monostate>(m_Buffers)) {
        Logger::WARNING("[DynamicBuffer] Trying to build a constructed dynamic buffer.");
        return true;
    }

    // use a vector to workaround arrays not accepting default constructible values
    std::vector<Buffer> buffers;
    buffers.reserve(detail::MAX_FRAMES_IN_FLIGHT);

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

    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        Buffer &buffer = buffers.emplace_back<Buffer::Info>({
            .usage = usage | Buffer::BufferUsageFlagBits::eTransferSrc | Buffer::BufferUsageFlagBits::eTransferDst,
            .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
        });

        if (!buffer.build(size)) {
            for (auto &buffer : buffers) {
                buffer.destroy();
            }

            return false;
        }
    }

    // use pack expansion to create an array from this vector
    auto makeArray = []<std::size_t... I>(const std::vector<Buffer> &buffers, std::index_sequence<I...>) {
        return std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT>{buffers[I]...};
    };

    m_Buffers = makeArray(buffers, std::make_index_sequence<detail::MAX_FRAMES_IN_FLIGHT>{});

    return true;
}

bool DynamicBuffer::destroy() {
    if (std::holds_alternative<std::monostate>(m_Buffers)) {
        Logger::WARNING("[DynamicBuffer] Trying to destroy a destructed dynamic buffer.");
        return true;
    }

    std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT> &buffers = std::get<std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT>>(m_Buffers);

    bool success = true;
    for (auto &buffer : buffers) {
        success &= buffer.destroy();
    }

    if (success) {
        m_Buffers = std::monostate();
    }

    return success;
}

bool DynamicBuffer::resize(const RenderContext &context, std::uint64_t size) {
    std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT> &buffers = std::get<std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT>>(m_Buffers);

    Buffer buffer = buffers[context.frameInFlight].getInfo();

    // create new buffer
    if (!buffer.build(size)) {
        Logger::ERROR("[DynamicBuffer] Failed to create new dynamic buffers.");
        return false;
    }

    // copy old data to the new buffer
    std::vector<vk::BufferCopy> copies = {{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = glm::min(buffers[context.frameInFlight].getData().bufferInfo.size, buffer.getData().bufferInfo.size),
    }};

    buffer.copy(context.command, buffers[context.frameInFlight], copies);

    // mark old buffer for deferred deletion and update
    context.deletionQueue->enqueue(std::move(buffers[context.frameInFlight]));
    buffers[context.frameInFlight] = buffer;

    notifyCallbacks<OnDynamicBufferRealloc>({
        .buffer = this,
        .context = context,
    });

    return true;
}

bool DynamicBuffer::update(const RenderContext &context, const std::span<const std::byte> &bytes, std::uint64_t offset) {
    std::uint64_t requiredSize = offset + bytes.size();

    if (getSize(context) < requiredSize) {
        resize(context, requiredSize);
    }

    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_Buffers), "[DynamicBuffer] Buffer has not been allocated.");
    const std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT> &buffers = std::get<std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT>>(m_Buffers);

    // TODO this doesnt have to happen on the transfer queue
    return context.systems.transfer->map(buffers[context.frameInFlight], bytes, offset);
}

const DynamicBuffer::Info &DynamicBuffer::getInfo() const {
    return m_Info;
}

std::size_t DynamicBuffer::getSize(const RenderContext &context) const {
    const std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT> &buffers = std::get<std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT>>(m_Buffers);

    return buffers[context.frameInFlight].getData().bufferInfo.size;
}

const std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT> &DynamicBuffer::getBuffers() const {
    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_Buffers), "[DynamicBuffer] Buffer has not been allocated.");
    return std::get<std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT>>(m_Buffers);
}

} // namespace Physbuzz
