#pragma once

#include "box.hpp"

class PhysicsBox : public Box {
  public:
    PhysicsBox(glm::vec2 position, float width, float height, float mass) : Box(position, width, height) {
        Box::identifier = Objects::PhysicsBox;
        dynamics.mass = mass;
    }
};
