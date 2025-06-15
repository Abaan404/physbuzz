#include "mesh.hpp"

namespace Physbuzz {

Mesh::Mesh(const MeshInfo &info)
    : m_Info(info) {}

bool Mesh::build() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, texCoords)));
    glEnableVertexAttribArray(2);
    unbind();

    return true;
}

bool Mesh::destroy() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    return true;
}

void Mesh::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_Info.vertices.size(), m_Info.vertices.data(), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * m_Info.indices.size(), m_Info.indices.data(), GL_STREAM_DRAW);
}

void Mesh::draw() const {
    bind();
    glDrawElements(GL_TRIANGLES, sizeof(Index) * m_Info.indices.size(), GL_UNSIGNED_INT, 0);
    unbind();
}

void Mesh::unbind() const {
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::update(const MeshInfo &info) {
    m_Info = info;
}

const MeshInfo &Mesh::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
