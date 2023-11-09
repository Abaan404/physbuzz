#pragma once

#include "object.hpp"

class Circle : public GameObject {
  public:
    Circle(glm::vec2 position, float radius) : GameObject(Objects::Circle, position), radius(radius) {
        GameObject::rect = {position.x - radius, position.y - radius, 2 * radius, 2 * radius};
    }

    float radius;
};
