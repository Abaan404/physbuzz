#pragma once

#include "object.hpp"

class Box : public GameObject {
  public:
    Box(glm::vec2 position, float width, float height, SDL_Color color) : GameObject(Objects::Box, position, color) {
        this->min = glm::vec2(position[0] - (width / 2.0f), position[1] - (height / 2.0f));
        this->max = glm::vec2(position[0] + (width / 2.0f), position[1] + (height / 2.0f));
    };

    // AABB variables
    glm::vec2 min;
    glm::vec2 max;
};
