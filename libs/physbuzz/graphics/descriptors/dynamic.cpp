#include "dynamic.hpp"

#include "../../app/deletion.hpp"
#include "../../events/descriptor.hpp"

namespace Physbuzz {

DynamicBuffer::DynamicBuffer(const Info &info)
    : m_Info(info), m_RingData(std::monostate()) {}

bool DynamicBuffer::build(std::uint64_t size) {
    if (!std::holds_alternative<std::monostate>(m_RingData)) {
        Logger::WARNING("[DynamicBuffer] Trying to build a constructed dynamic buffer.");
        return true;
    }

    // use a vector to workaround arrays not accepting default constructible values
    std::vector<Data> ringData;
    ringData.reserve(detail::MAX_FRAMES_IN_FLIGHT);

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
        Data &data = ringData.emplace_back<Data>({
            .buffer = {{
                .usage = usage | Buffer::BufferUsageFlagBits::eTransferSrc | Buffer::BufferUsageFlagBits::eTransferDst,
                .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
            }},
        });

        if (!data.buffer.build(size)) {
            for (auto &buffer : ringData) {
                data.buffer.destroy();
            }

            return false;
        }
    }

    // use pack expansion to create an array from this vector
    auto makeArray = []<std::size_t... I>(const std::vector<Data> &data, std::index_sequence<I...>) {
        return std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>{data[I]...};
    };

    m_RingData = makeArray(ringData, std::make_index_sequence<detail::MAX_FRAMES_IN_FLIGHT>{});

    return true;
}

bool DynamicBuffer::destroy() {
    if (std::holds_alternative<std::monostate>(m_RingData)) {
        Logger::WARNING("[DynamicBuffer] Trying to destroy a destructed dynamic buffer.");
        return true;
    }

    std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);

    bool success = true;
    for (auto &data : ringData) {
        success &= data.buffer.destroy();
    }

    if (success) {
        m_RingData = std::monostate();
    }

    return success;
}

bool DynamicBuffer::resize(const RenderContext &context, std::uint64_t size) {
    std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);

    Buffer buffer = ringData[context.frameInFlight].buffer.getInfo();

    // create new buffer
    if (!buffer.build(size)) {
        Logger::ERROR("[DynamicBuffer] Failed to create new dynamic buffers.");
        return false;
    }

    // copy old data to the new buffer
    std::vector<vk::BufferCopy> copies = {{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = glm::min(ringData[context.frameInFlight].buffer.getData().bufferInfo.size, buffer.getData().bufferInfo.size),
    }};

    buffer.copy(context.command, ringData[context.frameInFlight].buffer, copies);

    // mark old buffer for deferred deletion and update
    context.deletionQueue->enqueue(std::move(ringData[context.frameInFlight].buffer));

    ringData[context.frameInFlight] = {
        .buffer = buffer,
    };

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

    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_RingData), "[DynamicBuffer] Buffer has not been allocated.");
    const std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);

    // TODO this doesnt have to happen on the transfer queue
    return context.systems.transfer->map(ringData[context.frameInFlight].buffer, bytes, offset);
}

const DynamicBuffer::Info &DynamicBuffer::getInfo() const {
    return m_Info;
}

std::size_t DynamicBuffer::getSize(const RenderContext &context) const {
    const std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);

    return ringData[context.frameInFlight].buffer.getData().bufferInfo.size;
}

const std::array<DynamicBuffer::Data, detail::MAX_FRAMES_IN_FLIGHT> &DynamicBuffer::getRingData() const {
    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_RingData), "[DynamicBuffer] Buffer has not been allocated.");
    return std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);
}

} // namespace Physbuzz
