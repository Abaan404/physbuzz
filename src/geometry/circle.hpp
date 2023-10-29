#pragma once

#include "object.hpp"

class Circle : public GameObject {
  public:
    Circle(float x, float y, float radius, Mask mask) : GameObject(Objects::Circle, x, y, mask), radius(radius) {}
    float radius;

    bool collides(GameObject *object) override { return false; }
};
