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

    success &= batch.add(m_Vertices, std::move(vertices), 0);
    success &= batch.add(m_Indices, std::move(indices), 0);

    return success;
}

void Mesh::draw(const RenderContext &context, std::uint32_t instanceCount, std::uint32_t meshOffset) const {
    ZoneScopedN("Mesh/Draw");

    const Buffer::Data &vertices = m_Vertices.getData();
    const Buffer::Data &indices = m_Indices.getData();

    context.command.bindVertexBuffers(0, vertices.buffer, {0});
    context.command.bindIndexBuffer(indices.buffer, 0, vk::IndexType::eUint32);

    for (std::size_t submeshIdx = 0; submeshIdx < m_Info.submeshes.size(); submeshIdx++) {
        const SubMesh &submesh = m_Info.submeshes[submeshIdx];

        // each submesh is packed as SoA objects, calculate baseInstance accordingly
        // final offset is calculated as:
        //  objectIdx = instanceId + submeshIdx * instanceCount + meshOffset_(n-1)
        context.command.drawIndexed(
            submesh.indexCount,
            instanceCount,
            submesh.firstIndex,
            submesh.vertexOffset,
            submeshIdx * instanceCount + meshOffset);
    }
}

const Mesh::Info &Mesh::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
