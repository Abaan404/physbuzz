#pragma once

#include "object.hpp"

class Circle : public GameObject {
  public:
    Circle(int x, int y, int radius, Mask mask) : GameObject(Objects::Circle, x, y, mask), radius(radius) {}
    int radius;

    bool collides(GameObject *object) override { return false; }
};
