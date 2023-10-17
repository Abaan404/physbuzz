#pragma once

#include "object.hpp"
#include <SDL2/SDL_render.h>

class AABB : public Object {
  public:
    AABB(int x, int y, int width, int height, Mask mask);

    int width;
    int height;

    bool contains(int px, int py);
};
