#pragma once

#include "SDL_render.h"

class Object {
  public:
    SDL_Texture *texture;
    SDL_Rect rect;

    int x;
    int y;

    virtual void render_texture(SDL_Renderer *renderer, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) = 0;
};
