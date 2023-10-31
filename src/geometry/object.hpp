#pragma once

#include <SDL2/SDL_render.h>

struct Color {
    Uint8 R;
    Uint8 G;
    Uint8 B;
    Uint8 A;
};

enum class Objects {
    Unknown = -1,
    Box,
    PhysicsBox,
    Circle,
    PhysicsCircle
};

class GameObject {
  public:
    GameObject(Objects identifier, float x, float y, Color color) : x(x), y(y), color(color), identifier(identifier) {};

    float x;
    float y;

    SDL_Texture *texture;
    SDL_FRect rect;
    Color color;

    Objects identifier = Objects::Unknown;
    virtual bool collides(GameObject *object) = 0;
};
