#pragma once

#include "object.hpp"

class Circle : public GameObject {
  public:
    Circle(glm::vec2 position, float radius, SDL_Color color) : GameObject(Objects::Circle, position, color), radius(radius) {}
    float radius;
};
