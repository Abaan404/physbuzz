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

    success &= m_Vertices.build(m_Info.vertexCount * sizeof(std::byte) * m_Info.description->getInfo().size);
    success &= m_Indices.build(m_Info.indexCount * sizeof(Index));

    if (!success) {
        destroy();
        return false;
    }

    return true;
}

bool Mesh::destroy() {
    bool success = true;

    success &= m_Vertices.destroy();
    success &= m_Indices.destroy();

    return success;
}

bool Mesh::write(std::vector<std::byte> &&vertices, std::vector<std::byte> &&indices, TransferBatch &batch) {
    bool success = true;

    success &= batch.add(m_Vertices.getData().buffer, std::move(vertices), 0);
    success &= batch.add(m_Indices.getData().buffer, std::move(indices), 0);

    return success;
}

void Mesh::bind(const RenderContext &context) const {
    ZoneScopedN("Mesh/Draw");

    context.command.bindVertexBuffers(0, m_Vertices.getData().buffer.getData().buffer, {0});
    context.command.bindIndexBuffer(m_Indices.getData().buffer.getData().buffer, 0, vk::IndexType::eUint32);
}

const Mesh::Info &Mesh::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
