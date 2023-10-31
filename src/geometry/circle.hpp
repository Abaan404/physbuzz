#pragma once

#include "object.hpp"

class Circle : public GameObject {
  public:
    Circle(float x, float y, float radius, Color color) : GameObject(Objects::Circle, x, y, color), radius(radius) {}
    float radius;

    bool collides(GameObject *object) override { return false; }
};
