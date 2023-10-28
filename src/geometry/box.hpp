#pragma once

#include "object.hpp"
#include <SDL2/SDL_render.h>
#include <glm/glm.hpp>

class Box : public GameObject {
  public:
    Box(int x, int y, int width, int height, Mask mask);

    // AABB variables
    glm::ivec2 min;
    glm::ivec2 max;
};
