#include "mesh.hpp"

namespace Physbuzz {

MeshComponent::MeshComponent(std::vector<glm::uvec3> indices, std::vector<glm::vec3> vertices) : indices(indices),
                                                                                                 vertices(vertices),
                                                                                                 m_OriginalVertices(vertices) {
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const std::size_t next = (i + 1) % vertices.size();                        // cycle next vertex
        const glm::vec3 tangent = vertices[next] - vertices[i];                    // get the tangent
        const glm::vec3 normal = glm::cross(tangent, glm::vec3(0.0f, 0.0f, 1.0f)); // cross prod for normal

        normals.emplace_back(glm::normalize(normal));
    }
}

MeshComponent::~MeshComponent() {}

const std::vector<glm::vec3> &MeshComponent::getOriginalVertices() const {
    return m_OriginalVertices;
}

void MeshComponent::renormalize() {
    // Renderer::normalize will normalize the vertex with context to the current surface
    m_Scaled = false;
}

RenderComponent::RenderComponent() {}

RenderComponent::RenderComponent(const RenderComponent &other) {
    copy(other);
}

RenderComponent &RenderComponent::operator=(const RenderComponent &other) {
    if (this != &other) {
        copy(other);
    }
    return *this;
}

void RenderComponent::copy(const RenderComponent &other) {
    VBO = other.VBO;
    EBO = other.EBO;
    VAO = other.VAO;

    m_Program = other.m_Program;

    gluTime = other.gluTime;
    gluTimedelta = other.gluTimedelta;
    gluResolution = other.gluResolution;
}

RenderComponent::~RenderComponent() {}

void RenderComponent::build() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
}

void RenderComponent::destroy() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void RenderComponent::setProgram(unsigned int program) {
    m_Program = program;

    // common uniforms for every shader
    gluTime = glGetUniformLocation(program, "u_time");
    gluResolution = glGetUniformLocation(program, "u_resolution");
}

std::uint32_t RenderComponent::getProgram() const {
    return m_Program;
}

} // namespace Physbuzz
