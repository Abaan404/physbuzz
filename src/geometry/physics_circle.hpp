#pragma once

#include "circle.hpp"

class PhysicsCircle : public Circle {
  public:
    PhysicsCircle(glm::vec2 position, float radius, SDL_Color color) : Circle(position, radius, color) {
        Circle::identifier = Objects::PhysicsCircle;
    }
};
