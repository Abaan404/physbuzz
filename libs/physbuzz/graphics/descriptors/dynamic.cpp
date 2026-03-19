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

    Buffer::UsageFlags usage;
    switch (m_Info.type) {
    case Type::ConstantDynamic:
    case Type::Constant:
        usage = Buffer::UsageFlagBits::eUniformBuffer;
        break;

    case Type::StructuredDynamic:
    case Type::Structured:
        usage = Buffer::UsageFlagBits::eStorageBuffer;
        break;

    case Type::Indirect:
        usage = Buffer::UsageFlagBits::eIndirectBuffer | Buffer::UsageFlagBits::eStorageBuffer;
        break;
    }

    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        Buffer buffer = {{
            .usage = usage | Buffer::UsageFlagBits::eTransferSrc | Buffer::UsageFlagBits::eTransferDst,
            .memoryUsage = Buffer::MemoryUsage::GPUOnly,
        }};

        if (!buffer.build(size)) {
            for (auto &data : ringData) {
                data.buffer.destroy();
            }

            Logger::ERROR("[DynamicBuffer] Failed to build buffers.");
            return false;
        }

        ringData.emplace_back<Data>({
            .buffer = buffer,
        });
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

bool DynamicBuffer::rebuild(const RenderContext &context, std::uint64_t size) {
    Data &data = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData)[context.frameInFlight];

    Buffer buffer = data.buffer.getInfo();

    // create new buffer
    if (!buffer.build(size)) {
        Logger::ERROR("[DynamicBuffer] Failed to create new dynamic buffers.");
        return false;
    }

    // mark old buffer for deferred deletion and update
    context.deletionQueue->enqueue(std::move(data.buffer));

    data = {
        .buffer = buffer,
    };

    notifyCallbacks<OnDynamicBufferRebuild>({
        .buffer = this,
        .context = context,
    });

    return true;
}

bool DynamicBuffer::update(const RenderContext &context, const std::span<const std::byte> &bytes, std::uint64_t offset) {
    TracyVkZone(context.tracy, context.command, "DynamicBuffer/Update");

    std::uint64_t requiredSize = offset + bytes.size();

    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_RingData), "[DynamicBuffer] Buffer has not been allocated.");

    PBZ_ASSERT(
        requiredSize <= getSize(context.frameInFlight),
        std::format("[DynamicBuffer] Cannot update buffer with insufficient size ({}) and offset ({}) with size ({})", bytes.size(), offset, getSize(context.frameInFlight)));

    const Data &data = getRingData()[context.frameInFlight];
    return data.buffer.map(context.command, context.deletionQueue, bytes, offset);
}

const DynamicBuffer::Info &DynamicBuffer::getInfo() const {
    return m_Info;
}

const std::array<DynamicBuffer::Data, detail::MAX_FRAMES_IN_FLIGHT> &DynamicBuffer::getRingData() const {
    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_RingData), "[DynamicBuffer] Buffer has not been allocated.");
    return std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);
}

std::size_t DynamicBuffer::getSize(std::uint32_t frameInFlight) const {
    PBZ_ASSERT(frameInFlight < detail::MAX_FRAMES_IN_FLIGHT, "[DynamicBuffer] Invalid frame in flight");
    const Data &data = getRingData()[frameInFlight];

    return data.buffer.getData().bufferInfo.size;
}

} // namespace Physbuzz
