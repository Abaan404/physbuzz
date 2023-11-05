#pragma once

#include <SDL2/SDL_render.h>

enum class Objects {
    Unknown = -1,
    Box,
    PhysicsBox,
    Circle,
    PhysicsCircle
};

class GameObject {
  public:
    GameObject(Objects identifier, float x, float y, SDL_Color color) : x(x), y(y), color(color), identifier(identifier){};

    float x;
    float y;

    SDL_Texture *texture;
    SDL_FRect rect;
    SDL_Color color;

    Objects identifier = Objects::Unknown;
};
