#include "mesh.hpp"

#include "defines.hpp"
#include "transfer.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

VertexDescription::VertexDescription(const Info &info)
    : m_Info(info) {
    vk::VertexInputBindingDescription binding = {
        .binding = m_Info.binding,
        .stride = m_Info.size,
        .inputRate = m_Info.inputRate,
    };

    std::vector<vk::VertexInputAttributeDescription> attributes;
    attributes.reserve(m_Info.attributes.size());

    for (std::size_t i = 0; i < m_Info.attributes.size(); i++) {
        attributes.emplace_back<vk::VertexInputAttributeDescription>({
            .location = static_cast<std::uint32_t>(i),
            .binding = m_Info.binding,
            .format = m_Info.attributes[i].format,
            .offset = m_Info.attributes[i].offset,
        });
    }

    m_Data = {
        .binding = binding,
        .attributes = attributes,
    };
}

const VertexDescription::Info &VertexDescription::getInfo() const {
    return m_Info;
}

const VertexDescription::Data &VertexDescription::getData() const {
    return m_Data;
}

bool Mesh::build() {
    bool success = true;

    std::uint64_t vertexSize = m_Info.vertexCount * sizeof(std::byte) * m_Info.description->getInfo().size;
    std::uint64_t indexSize = m_Info.indexCount * sizeof(Index);

    success &= m_Mesh.build(vertexSize + indexSize);

    if (!success) {
        destroy();
        return false;
    }

    return true;
}

bool Mesh::destroy() {
    bool success = true;

    success &= m_Mesh.destroy();

    return success;
}

bool Mesh::write(std::vector<std::byte> &&vertices, std::vector<std::byte> &&indices, TransferBatch &batch) {
    bool success = true;

    std::uint64_t vertexSize = m_Info.vertexCount * sizeof(std::byte) * m_Info.description->getInfo().size;

    // I'm gonna keep a cpu copy of vertices and indices for now
    // TODO since static buffer is just a wrapper around a plain buffer it should be replaced
    // with just a buffer as it serves no real benefit, should then allow finer control on how
    // memory should be mapped/cpu visible for things like this or a material buffer
    m_Vertices = vertices;
    m_Indices = indices;

    success &= batch.add(m_Mesh, std::move(vertices), 0);
    success &= batch.add(m_Mesh, std::move(indices), vertexSize);

    return success;
}

void Mesh::bind(const RenderContext &context) const {
    ZoneScopedN("Mesh/Draw");

    std::uint64_t vertexSize = m_Info.vertexCount * sizeof(std::byte) * m_Info.description->getInfo().size;

    context.command.bindVertexBuffers(0, m_Mesh.getData().buffer, {0});
    context.command.bindIndexBuffer(m_Mesh.getData().buffer, vertexSize, vk::IndexType::eUint32);
}

const std::vector<std::byte> &Mesh::getVertices() const {
    return m_Vertices;
}

const std::vector<std::byte> &Mesh::getIndices() const {
    return m_Indices;
}

const Mesh::Info &Mesh::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
