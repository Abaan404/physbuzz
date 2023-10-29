#pragma once

#include "object.hpp"
#include <SDL2/SDL_render.h>
#include <glm/glm.hpp>

class Box : public GameObject {
  public:
    Box(int x, int y, int width, int height, Mask mask) : GameObject(Objects::Box, x, y, mask) {
        this->min = glm::vec2(x - (width >> 1), y - (height >> 1));
        this->max = glm::vec2(x + (width >> 1), y + (height >> 1));
    };

    // AABB variables
    glm::ivec2 min;
    glm::ivec2 max;

    bool collides(GameObject *object) override { return false; }
};
