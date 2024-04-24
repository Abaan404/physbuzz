#include "mesh.hpp"

MeshComponent::MeshComponent() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
}

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
    // probably not ideal
    VBO = other.VBO;
    EBO = other.EBO;
    VAO = other.VAO;

    vertices = other.vertices;
    indices = other.indices;
    program = other.program;
    normalized = other.normalized;

    glu_time = other.glu_time;
    glu_timedelta = other.glu_timedelta;
    glu_resolution = other.glu_resolution;
}

MeshComponent::~MeshComponent() {
    // i hate this but i cant think of anything else
    if (preserve_buffers)
        return;

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void MeshComponent::set_vertex(std::vector<glm::vec3> &vertex_buffer) {
    vertices.clear();
    vertices.reserve(vertex_buffer.size() * 3);
    for (auto &vertex : vertex_buffer) {
        vertices.emplace_back(vertex.x);
        vertices.emplace_back(vertex.y);
        vertices.emplace_back(vertex.z);
    }
}

void MeshComponent::set_index(std::vector<glm::uvec3> &index_buffer) {
    indices.clear();
    indices.reserve(index_buffer.size() * 3);
    for (auto &index : index_buffer) {
        indices.emplace_back(index.x);
        indices.emplace_back(index.y);
        indices.emplace_back(index.z);
    }
}

void MeshComponent::set_program(unsigned int program) {
    this->program = program;
    glUseProgram(program);

    // common uniforms for every shader
    glu_time = glGetUniformLocation(program, "u_time");
    glu_resolution = glGetUniformLocation(program, "u_resolution");
}

GLuint MeshComponent::get_program() {
    return program;
}

std::vector<glm::vec3> MeshComponent::get_vertex() {
    std::vector<glm::vec3> vertex_buffer;
    vertex_buffer.reserve(vertices.size() / 3);

    int idx = 0;
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex += 3) {
        vertex_buffer.emplace_back(*vertex, *(vertex + 1), *(vertex + 2));
    }
    return vertex_buffer;
}

std::vector<glm::uvec3> MeshComponent::get_index() {
    std::vector<glm::uvec3> index_buffer;
    index_buffer.reserve(indices.size() / 3);

    int idx = 0;
    for (auto index = indices.begin(); index != indices.end(); index += 3) {
        index_buffer.emplace_back(*index, *(index + 1), *(index + 2));
    }
    return index_buffer;
}
