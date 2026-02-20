#include "static.hpp"

#include "../../app/application.hpp"
#include "../../app/deletion.hpp"
#include "../../debug/macros.hpp"
#include "../../events/descriptor.hpp"
#include "../defines.hpp"

namespace Physbuzz {

StaticBuffer::StaticBuffer() {}

bool StaticBuffer::build(std::uint64_t size) {
    if (m_Data.address != 0) {
        Logger::WARNING("[StaticBuffer] Trying to build a constructed static buffer.");
        return true;
    }

    Buffer buffer = {{
        .usage = Buffer::UsageFlagBits::eTransferSrc | Buffer::UsageFlagBits::eTransferDst | Buffer::UsageFlagBits::eShaderDeviceAddress,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};

    if (!buffer.build(size)) {
        Logger::ERROR("[StaticBuffer] Failed to build buffer.");
        return false;
    }

    m_Data = {
        .buffer = buffer,
        .address = App::Device.getBufferAddress({
            .buffer = buffer.getData().buffer,
        }),
    };

    return true;
}

bool StaticBuffer::destroy() {
    if (m_Data.address == 0) {
        Logger::WARNING("[StaticBuffer] Trying to destroy a destructed static buffer.");
        return true;
    }

    if (!m_Data.buffer.destroy()) {
        return false;
    }

    m_Data = {
        .buffer = {{}},
        .address = 0,
    };

    return true;
}

bool StaticBuffer::rebuild(const RenderContext &context, std::uint64_t size) {
    Buffer buffer = m_Data.buffer.getInfo();

    // create new buffer
    if (!buffer.build(size)) {
        Logger::ERROR("[StaticBuffer] Failed to create new shader buffers.");
        return false;
    }

    // mark old buffer for deferred deletion and update
    context.deletionQueue->enqueue(std::move(m_Data.buffer));

    m_Data = {
        .buffer = buffer,
        .address = App::Device.getBufferAddress({
            .buffer = buffer.getData().buffer,
        }),
    };

    notifyCallbacks<OnStaticBufferRebuild>({
        .buffer = this,
    });

    return true;
}

bool StaticBuffer::update(const RenderContext &context, const std::span<const std::byte> &bytes, std::uint64_t offset) {
    std::uint64_t requiredSize = offset + bytes.size();

    PBZ_ASSERT(m_Data.address != 0, "[StaticBuffer] Buffer has not been allocated.");

    PBZ_ASSERT(
        requiredSize <= getSize(),
        std::format("[StaticBuffer] Cannot update buffer with insufficient size ({}) and offset ({}) with size ({})", bytes.size(), offset, getSize()));

    return m_Data.buffer.map(context.command, context.deletionQueue, bytes, offset);
}

std::size_t StaticBuffer::getSize() const {
    return m_Data.buffer.getData().bufferInfo.size;
}

const StaticBuffer::Data &StaticBuffer::getData() const {
    return m_Data;
}

} // namespace Physbuzz
