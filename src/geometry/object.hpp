#pragma once

#include <SDL2/SDL.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

enum class Objects {
    Unknown = -1,
    Box,
    PhysicsBox,
    Circle,
    PhysicsCircle
};

struct DynamicProperties {
    float mass = 0.0f;
    float intertia = 0.0f;
    glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
    glm::vec2 acceleration = glm::vec2(0.0f, 0.0f);
};

class GameObject {
  public:
    GameObject(Objects identifier, glm::vec2 position) : position(position), identifier(identifier) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &EBO);
    };

    ~GameObject() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &EBO);
    }

    unsigned int VAO, EBO;
    unsigned int program;

    glm::vec2 position;
    float rotation;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    SDL_Texture *texture;
    SDL_FRect rect;

    DynamicProperties dynamics;
    Objects identifier = Objects::Unknown;

    void set_vertex(std::vector<glm::vec3> vertex_buffer) {
        int idx = 0;
        vertices.resize(vertex_buffer.size() * 3);
        for (auto vertex = vertex_buffer.begin(); vertex != vertex_buffer.end(); vertex++) {
            vertices[idx++] = vertex->x;
            vertices[idx++] = vertex->y;
            vertices[idx++] = vertex->z;
        }
    }

    void set_index(std::vector<glm::uvec3> index_buffer) {
        int idx = 0;
        indices.resize(index_buffer.size() * 3);
        for (auto index = index_buffer.begin(); index != index_buffer.end(); index++) {
            indices[idx++] = index->x;
            indices[idx++] = index->y;
            indices[idx++] = index->z;
        }
    }
};
