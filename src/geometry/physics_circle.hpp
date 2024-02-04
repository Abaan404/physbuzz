#pragma once

#include "circle.hpp"

class PhysicsCircle : public Circle {
  public:
    PhysicsCircle(glm::vec2 position, float radius, float mass) : Circle(position, radius) {
        GameObject::identifier = Objects::PhysicsCircle;
        GameObject::dynamics.mass = mass;
    }
};
