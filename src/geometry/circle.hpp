#pragma once

#include "object.hpp"

class Circle : public GameObject {
  public:
    Circle(glm::vec2 position, float radius);

    float radius;
};
