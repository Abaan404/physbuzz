#include "object.hpp"
#include <glad/gl.h>

GameObject::GameObject(Objects identifier, glm::vec2 position) : position(position), identifier(identifier) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
};

GameObject::~GameObject() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
}

void GameObject::set_vertex(std::vector<glm::vec3> vertex_buffer) {
    int idx = 0;
    vertices.resize(vertex_buffer.size() * 3);
    for (auto vertex = vertex_buffer.begin(); vertex != vertex_buffer.end(); vertex++) {
        vertices[idx++] = vertex->x;
        vertices[idx++] = vertex->y;
        vertices[idx++] = vertex->z;
    }
}

void GameObject::set_index(std::vector<glm::uvec3> index_buffer) {
    int idx = 0;
    indices.resize(index_buffer.size() * 3);
    for (auto index = index_buffer.begin(); index != index_buffer.end(); index++) {
        indices[idx++] = index->x;
        indices[idx++] = index->y;
        indices[idx++] = index->z;
    }
}
