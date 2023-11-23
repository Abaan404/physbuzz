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

struct DynamicProperties {
    float intertia;
    glm::vec2 velocity;
    glm::vec2 acceleration;
};

class GameObject {
  public:
    GameObject(Objects identifier, glm::vec2 position) : position(position), identifier(identifier){};

    glm::vec2 position;
    float rotation;

    SDL_Texture *texture;
    SDL_FRect rect;

    DynamicProperties dynamics;
    Objects identifier = Objects::Unknown;
};

