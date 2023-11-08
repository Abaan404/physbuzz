#pragma once

#include <SDL2/SDL_render.h>
#include <glm/glm.hpp>

enum class Objects {
    Unknown = -1,
    Box,
    PhysicsBox,
    Circle,
    PhysicsCircle
};

class GameObject {
  public:
    GameObject(Objects identifier, glm::vec2 position, SDL_Color color) : position(position), color(color), identifier(identifier){};

    glm::vec2 position;
    float rotation;

    SDL_Texture *texture;
    SDL_FRect rect;
    SDL_Color color;

    Objects identifier = Objects::Unknown;
};

class DynamicObject {
  public:
    float intertia;
    glm::vec2 velocity;
    glm::vec2 acceleration;
};
