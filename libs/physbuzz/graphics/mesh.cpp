#include "mesh.hpp"

#include "defines.hpp"
#include "transfer.hpp"
#include <tracy/Tracy.hpp>

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

bool Mesh::build() {
    bool success = true;

    success &= m_Vertex.build(m_Info.vertexCount * sizeof(std::byte) * m_Info.description->getInfo().size);
    success &= m_Index.build(m_Info.indexCount * sizeof(Index));

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

bool Mesh::write(std::vector<std::byte> &&vertices, std::vector<std::byte> &&indices, TransferBatch &batch) const {
    bool success = true;

    success &= batch.add(m_Vertex, std::move(vertices), 0);
    success &= batch.add(m_Index, std::move(indices), 0);

    return success;
}

void Mesh::draw(const RenderContext &context, std::uint32_t instances, std::uint32_t object) const {
    ZoneScopedN("Mesh/Draw");

    const Buffer::Data &vertex = m_Vertex.getData();
    const Buffer::Data &index = m_Index.getData();

    context.command.bindVertexBuffers(0, vertex.buffer, {0});
    context.command.bindIndexBuffer(index.buffer, 0, vk::IndexType::eUint32);

    context.command.drawIndexed(m_Info.indexCount, instances, 0, 0, object);
}

const Mesh::Info &Mesh::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
