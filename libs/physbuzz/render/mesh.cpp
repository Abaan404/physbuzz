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
    if (m_Vertex.has_value()) {
        Logger::WARNING("[Mesh] Trying to build a constructed mesh.");
        return true;
    }

    if (m_Transfer == nullptr) {
        Logger::ERROR("[Mesh] Transfer is null.");
        return false;
    }

    std::optional<Buffer> stagingVertexBuffer = m_Transfer->createBuffer({
        .size = m_Vertices.size() * sizeof(m_Vertices[0]),
        .usage = Buffer::BufferUsageFlagBits::eTransferSrc,
        .properties = Buffer::MemoryPropertyFlagBits::eHostVisible | Buffer::MemoryPropertyFlagBits::eHostCoherent,
    });

    m_Vertex = m_Transfer->createBuffer({
        .size = m_Vertices.size() * sizeof(m_Vertices[0]),
        .usage = Buffer::BufferUsageFlagBits::eVertexBuffer | Buffer::BufferUsageFlagBits::eTransferDst,
        .properties = Buffer::MemoryPropertyFlagBits::eDeviceLocal,
    });

    std::optional<Buffer> stagingIndexBuffer = m_Transfer->createBuffer({
        .size = m_Indices.size() * sizeof(m_Indices[0]),
        .usage = Buffer::BufferUsageFlagBits::eTransferSrc,
        .properties = Buffer::MemoryPropertyFlagBits::eHostVisible | Buffer::MemoryPropertyFlagBits::eHostCoherent,
    });

    m_Index = m_Transfer->createBuffer({
        .size = m_Indices.size() * sizeof(m_Indices[0]),
        .usage = Buffer::BufferUsageFlagBits::eIndexBuffer | Buffer::BufferUsageFlagBits::eTransferDst,
        .properties = Buffer::MemoryPropertyFlagBits::eDeviceLocal,
    });

    if (!stagingVertexBuffer.has_value() || !m_Vertex.has_value() || !m_Index.has_value()) {
        if (stagingVertexBuffer.has_value()) {
            m_Transfer->eraseBuffer(stagingVertexBuffer.value());
        }

        destroy();
        return false;
    }

    bool success = true;

    success &= stagingVertexBuffer->map(m_Vertices);
    success &= m_Transfer->copy(stagingVertexBuffer.value(), m_Vertex.value(), stagingVertexBuffer->getInfo().size, true);

    success &= stagingIndexBuffer->map(m_Indices);
    success &= m_Transfer->copy(stagingIndexBuffer.value(), m_Index.value(), stagingIndexBuffer->getInfo().size, true);

    if (!success) {
        destroy();
        return false;
    }

    return true;
}

bool Mesh::destroy() {
    if (!m_Index.has_value() && !m_Vertex.has_value()) {
        Logger::WARNING("[Mesh] Trying to destroy a destructed mesh.");
        return true;
    }

    bool success = true;

    if (m_Index.has_value()) {
        success &= m_Transfer->eraseBuffer(m_Index.value());
    }

    if (m_Vertex.has_value()) {
        success &= m_Transfer->eraseBuffer(m_Vertex.value());
    }

    return success;
}

void Mesh::draw(const vk::CommandBuffer &commandBuffer) const {
    const Buffer::Data &vertex = m_Vertex->getData();
    const Buffer::Data &index = m_Index->getData();

    commandBuffer.bindVertexBuffers(0, vertex.buffer, {0});
    commandBuffer.bindIndexBuffer(index.buffer, 0, vk::IndexType::eUint16);

    commandBuffer.drawIndexed(m_Indices.size(), 1, 0, 0, 0);
}

const VertexDescription *Mesh::getDescription() const {
    return m_Description;
}

} // namespace Physbuzz
