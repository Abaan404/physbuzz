#pragma once

#include <SDL2/SDL_render.h>

struct Mask {
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
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
    GameObject(Objects identifier, int x, int y, Mask mask) : x(x), y(y), mask(mask), identifier(identifier) {};

    int x;
    int y;

    SDL_Texture *texture;
    SDL_Rect rect;
    Mask mask;

    Objects identifier = Objects::Unknown;
    virtual bool collides(GameObject *object) = 0;
};
