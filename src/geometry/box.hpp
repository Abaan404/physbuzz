#pragma once

#include "SDL_render.h"
#include "object.hpp"

class AABB : public Object {
  public:
    AABB(int x, int y, int width, int height);

    int width;
    int height;

    bool contains(int px, int py);
    void render_texture(SDL_Renderer *renderer, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) override;
};
