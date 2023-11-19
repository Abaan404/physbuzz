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

class GameObject {
  public:
    GameObject(Objects identifier, glm::vec2 position) : position(position), identifier(identifier) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    };

    GLuint VAO;
    GLuint program;

    glm::vec2 position;
    float rotation;
    std::vector<glm::vec3> vertices;

    SDL_Texture *texture;
    SDL_FRect rect;

    Objects identifier = Objects::Unknown;
};

class DynamicObject {
  public:
    float intertia;
    glm::vec2 velocity;
    glm::vec2 acceleration;
};
