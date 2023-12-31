#pragma once

#include "circle.hpp"

class PhysicsCircle : public Circle {
  public:
    PhysicsCircle(glm::vec2 position, float radius, float mass) : Circle(position, radius) {
        Circle::identifier = Objects::PhysicsCircle;
        dynamics.mass = mass;
    }
};
