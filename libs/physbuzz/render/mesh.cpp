#include "mesh.hpp"

#include "../app/application.hpp"

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

    m_VertexInputStateCreateInfo = {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &m_Binding,
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(m_Attributes.size()),
        .pVertexAttributeDescriptions = m_Attributes.data(),
    };
}

const VertexDescription::Info &VertexDescription::getInfo() const {
    return m_Info;
}

bool Mesh::build() {
    if (m_Transfer == nullptr) {
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

    std::span<const std::byte> vertices = {
        reinterpret_cast<const std::byte *>(m_Vertices.data()),
        m_Vertices.size() * sizeof(std::byte),
    };

    std::span<const std::byte> indices = {
        reinterpret_cast<const std::byte *>(m_Indices.data()),
        m_Indices.size() * sizeof(Index),
    };

    success &= m_Transfer->map(m_Vertex, vertices);
    success &= m_Transfer->map(m_Index, indices);

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

void Mesh::draw(const vk::CommandBuffer &commandBuffer) const {
    const Buffer::Data &vertex = m_Vertex.getData();
    const Buffer::Data &index = m_Index.getData();

    commandBuffer.bindVertexBuffers(0, vertex.buffer, {0});
    commandBuffer.bindIndexBuffer(index.buffer, 0, vk::IndexType::eUint32);

    commandBuffer.drawIndexed(m_Indices.size(), 1, 0, 0, 0);
}

const VertexDescription *Mesh::getDescription() const {
    return m_Description;
}

} // namespace Physbuzz
