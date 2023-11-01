#pragma once

#include "object.hpp"
#include <glm/glm.hpp>

class Box : public GameObject {
  public:
    Box(float x, float y, float width, float height, SDL_Color color) : GameObject(Objects::Box, x, y, color) {
        this->min = glm::vec2(x - (width / 2.0f), y - (height / 2.0f));
        this->max = glm::vec2(x + (width / 2.0f), y + (height / 2.0f));
    };

    // AABB variables
    glm::vec2 min;
    glm::vec2 max;

    void collides(GameObject *object) override {}
};
