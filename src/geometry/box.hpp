#pragma once

#include "object.hpp"

class Box : public GameObject {
  public:
    Box(glm::vec2 position, float width, float height) : GameObject(Objects::Box, position) {
        min = glm::vec2(position[0] - (width / 2.0f), position[1] - (height / 2.0f));
        max = glm::vec2(position[0] + (width / 2.0f), position[1] + (height / 2.0f));

        GameObject::rect = {min.x, min.y, width, height};
    };

    // AABB variables
    glm::vec2 min;
    glm::vec2 max;
};
