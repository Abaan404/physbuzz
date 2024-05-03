#include "mesh.hpp"

namespace Physbuzz {

MeshComponent::MeshComponent() {}

MeshComponent::MeshComponent(const MeshComponent &other) {
    copy(other);
}

MeshComponent &MeshComponent::operator=(const MeshComponent &other) {
    if (this != &other) {
        copy(other);
    }
    return *this;
}

void MeshComponent::copy(const MeshComponent &other) {
    VBO = other.VBO;
    EBO = other.EBO;
    VAO = other.VAO;

    m_Vertices = other.m_Vertices;
    m_Indices = other.m_Indices;
    m_ScreenVertices = other.m_ScreenVertices;
    m_Program = other.m_Program;

    gluTime = other.gluTime;
    gluTimedelta = other.gluTimedelta;
    gluResolution = other.gluResolution;
}

MeshComponent::~MeshComponent() {}

void MeshComponent::build() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
}

void MeshComponent::destroy() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void MeshComponent::redraw() {
    // defer recreating m_Vertices to an associated Renderer::render()
    m_Vertices.clear();
}

void MeshComponent::setVertex(std::vector<glm::vec3> &vertexBuffer) {
    m_ScreenVertices.clear();
    m_ScreenVertices.reserve(vertexBuffer.size() * 3);
    for (auto &vertex : vertexBuffer) {
        m_ScreenVertices.emplace_back(vertex.x);
        m_ScreenVertices.emplace_back(vertex.y);
        m_ScreenVertices.emplace_back(vertex.z);
    }

    redraw();
}

void MeshComponent::setIndex(std::vector<glm::uvec3> &indexBuffer) {
    m_Indices.clear();
    m_Indices.reserve(indexBuffer.size() * 3);
    for (auto &index : indexBuffer) {
        m_Indices.emplace_back(index.x);
        m_Indices.emplace_back(index.y);
        m_Indices.emplace_back(index.z);
    }
}

void MeshComponent::setProgram(unsigned int program) {
    m_Program = program;
    glUseProgram(program);

    // common uniforms for every shader
    gluTime = glGetUniformLocation(program, "u_time");
    gluResolution = glGetUniformLocation(program, "u_resolution");
}

GLuint MeshComponent::getProgram() const {
    return m_Program;
}

std::vector<glm::vec3> MeshComponent::getVertex() const {
    std::vector<glm::vec3> vertexBuffer;
    vertexBuffer.reserve(m_ScreenVertices.size() / 3);

    int idx = 0;
    for (auto vertex = m_ScreenVertices.begin(); vertex != m_ScreenVertices.end(); vertex += 3) {
        vertexBuffer.emplace_back(*vertex, *(vertex + 1), *(vertex + 2));
    }
    return vertexBuffer;
}

std::vector<glm::uvec3> MeshComponent::getIndex() const {
    std::vector<glm::uvec3> indexBuffer;
    indexBuffer.reserve(m_Indices.size() / 3);

    int idx = 0;
    for (auto index = m_Indices.begin(); index != m_Indices.end(); index += 3) {
        indexBuffer.emplace_back(*index, *(index + 1), *(index + 2));
    }
    return indexBuffer;
}

} // namespace Physbuzz
