#include "mesh.hpp"

#include "defines.hpp"

namespace Physbuzz {

VertexDescription::VertexDescription(const Info &info)
    : m_Info(info) {
    m_Binding = {
        .binding = m_Info.binding,
        .stride = m_Info.size,
        .inputRate = m_Info.inputRate,
    };

    m_Attributes.clear();
    for (std::size_t i = 0; i < m_Info.attributes.size(); i++) {
        m_Attributes.push_back({
            .location = static_cast<std::uint32_t>(i),
            .binding = m_Info.binding,
            .format = m_Info.attributes[i].format,
            .offset = m_Info.attributes[i].offset,
        });
    }
}

const VertexDescription::Info &VertexDescription::getInfo() const {
    return m_Info;
}

bool Mesh::build(const std::shared_ptr<Transfer> transfer) {
    if (transfer == nullptr) {
        Logger::ERROR("[Mesh] No transfer system provided for mesh.");
        return false;
    }

    bool success = true;

    success &= m_Vertex.build(m_Vertices.size() * sizeof(std::byte));
    success &= m_Index.build(m_Indices.size() * sizeof(Index));

    if (!success) {
        destroy();
        return false;
    }

    std::span<const std::byte> vertices = std::as_bytes(std::span(m_Vertices));
    std::span<const std::byte> indices = std::as_bytes(std::span(m_Indices));

    success &= transfer->map(m_Vertex, vertices, 0);
    success &= transfer->map(m_Index, indices, 0);

    if (!success) {
        destroy();
        return false;
    }

    return true;
}

bool Mesh::destroy() {
    bool success = true;

    success &= m_Vertex.destroy();
    success &= m_Index.destroy();

    return success;
}

void Mesh::draw(const RenderContext &context, std::uint32_t instances, std::uint32_t object) const {
    const Buffer::Data &vertex = m_Vertex.getData();
    const Buffer::Data &index = m_Index.getData();

    context.command.bindVertexBuffers(0, vertex.buffer, {0});
    context.command.bindIndexBuffer(index.buffer, 0, vk::IndexType::eUint32);

    context.command.drawIndexed(m_Indices.size(), instances, 0, 0, object);
}

const VertexDescription *Mesh::getDescription() const {
    return m_Description;
}

} // namespace Physbuzz
