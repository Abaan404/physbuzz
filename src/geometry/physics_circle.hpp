#pragma once

#include "circle.hpp"

class PhysicsCircle : public Circle {
  public:
    PhysicsCircle(float x, float y, float radius, SDL_Color color) : Circle(x, y, radius, color) {
        Circle::identifier = Objects::PhysicsCircle;
    }
};
