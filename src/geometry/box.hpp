#pragma once

#include "SDL_render.h"
#include "object.hpp"

class AABB : public Object {
  public:
    AABB(int x, int y, int width, int height, Mask mask);

    int width;
    int height;

    bool contains(int px, int py);
};
