#pragma once

#include <SDL2/SDL_render.h>

struct Mask {
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
};

class GameObject {
  public:
    SDL_Texture *texture;
    SDL_Rect rect;
    Mask mask;

    int x;
    int y;
};

class PhysicsObject {
  public:
    virtual bool collides(PhysicsObject *object) = 0;
};
