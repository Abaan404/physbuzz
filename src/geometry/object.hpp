#pragma once

#include "SDL_render.h"

class Object {
  public:
    int x;
    int y;

    virtual SDL_Texture *create_texture(SDL_Renderer *renderer, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) = 0;
};
