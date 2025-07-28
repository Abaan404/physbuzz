#include "mesh.hpp"

namespace Physbuzz {

VertexAttribute::VertexAttribute(const Info &info)
    : m_Info(info) {}

bool VertexAttribute::build() {
    if (VAO != 0 && VBO != 0 && EBO != 0) {
        Logger::ERROR("[VertexAttribute] Cannot create already built vertex attributes.");
        return false;
    }

    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);
    glCreateVertexArrays(1, &VAO);

    std::size_t i = 0;
    for (const auto &format : m_Info.attributes) {
        glEnableVertexArrayAttrib(VAO, i);
        glVertexArrayAttribBinding(VAO, i, 0);

        switch (format.type) {
        case Types::Byte:
        case Types::UnsignedByte:
        case Types::Short:
        case Types::UnsignedShort:
        case Types::Int:
        case Types::UnsignedInt:
            glVertexArrayAttribIFormat(VAO, i, format.size, static_cast<GLenum>(format.type), format.offset);
            break;

        case Types::HalfFloat:
        case Types::Fixed:
        case Types::Float:
            glVertexArrayAttribFormat(VAO, i, format.size, static_cast<GLenum>(format.type), GL_FALSE, format.offset);
            break;

        case Types::Double:
            glVertexArrayAttribLFormat(VAO, i, format.size, static_cast<GLenum>(format.type), format.offset);
            break;
        }
        i++;
    }

    glVertexArrayElementBuffer(VAO, EBO);
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, m_Info.size);

    return true;
}

bool VertexAttribute::destroy() {
    if (VAO == 0 && VBO == 0 && EBO == 0) {
        Logger::ERROR("[VertexAttribute] Cannot destroy already destructed vertex attributes.");
        return false;
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    return true;
}

const VertexAttribute::Info &VertexAttribute::getInfo() const {
    return m_Info;
}

bool Mesh::build() {
    return true;
}

bool Mesh::destroy() {
    return true;
}

void Mesh::draw() const {
    const VertexAttribute *attribute = m_Attribute.get();

    glNamedBufferData(attribute->VBO, m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);
    glNamedBufferData(attribute->EBO, m_Indices.size() * sizeof(Index), m_Indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(attribute->VAO);

    glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
}

const Resource<VertexAttribute> &Mesh::getAttribute() const {
    return m_Attribute;
}

const std::vector<std::byte> &Mesh::getVertices() const {
    return m_Vertices;
}

const std::vector<Index> &Mesh::getIndices() const {
    return m_Indices;
}

} // namespace Physbuzz
