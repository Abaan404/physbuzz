#pragma once

#include <SDL2/SDL_render.h>

struct Mask {
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
};

class Object {
  public:
    SDL_Texture *texture;
    SDL_Rect rect;

    int x;
    int y;

    Mask mask;
};
