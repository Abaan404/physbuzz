#include "static.hpp"

#include "../../app/application.hpp"
#include "../../app/deletion.hpp"
#include "../../debug/macros.hpp"
#include "../../events/descriptor.hpp"
#include "../defines.hpp"

namespace Physbuzz {

StaticBuffer::StaticBuffer(const Info &info)
    : m_Info(info) {}

bool StaticBuffer::build(std::uint64_t size) {
    if (m_Data.buffer.getData().address != 0) {
        Logger::WARNING("[StaticBuffer] Trying to build a constructed static buffer.");
        return true;
    }

    Buffer::UsageFlags usage = {};

    switch (m_Info.type) {
    case Type::Vertex:
        usage = Buffer::UsageFlags::eVertexBuffer;
        break;
    case Type::Index:
        usage = Buffer::UsageFlags::eIndexBuffer;
        break;
    case Type::None:
        break;
    }

    Buffer buffer = {{
        .usage = usage | Buffer::UsageFlags::eTransferSrc | Buffer::UsageFlags::eTransferDst | Buffer::UsageFlags::eShaderDeviceAddress,
        .memoryUsage = Buffer::MemoryUsage::GPUOnly,
    }};

    if (!buffer.build(size)) {
        Logger::ERROR("[StaticBuffer] Failed to build buffer.");
        return false;
    }

    m_Data = {
        .buffer = buffer,
    };

    return true;
}

bool StaticBuffer::destroy() {
    if (m_Data.buffer.getData().address == 0) {
        Logger::WARNING("[StaticBuffer] Trying to destroy a destructed static buffer.");
        return true;
    }

    if (!m_Data.buffer.destroy()) {
        return false;
    }

    m_Data = {
        .buffer = {{}},
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
    };

    notifyCallbacks<OnStaticBufferRebuild>({
        .buffer = this,
        .context = context,
    });

    return true;
}

bool StaticBuffer::update(const RenderContext &context, const std::span<const std::byte> &bytes, std::uint64_t offset) {
    TracyVkZone(context.tracy, context.command, "StaticBuffer/Update");

    std::uint64_t requiredSize = offset + bytes.size();

    PBZ_ASSERT(m_Data.buffer.getData().address != 0, "[StaticBuffer] Buffer has not been allocated.");

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
