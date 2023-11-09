#pragma once

#include "circle.hpp"

class PhysicsCircle : public DynamicObject, public Circle {
  public:
    PhysicsCircle(glm::vec2 position, float radius) : Circle(position, radius) {
        Circle::identifier = Objects::PhysicsCircle;
    }
};
