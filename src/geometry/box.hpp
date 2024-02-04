#pragma once

#include "object.hpp"

class Box : public GameObject {
  public:
    Box(glm::vec2 position, float width, float height);

    // AABB variables
    glm::vec2 min;
    glm::vec2 max;
};
