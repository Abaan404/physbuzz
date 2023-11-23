#pragma once

#include "box.hpp"

class PhysicsBox : public Box {
  public:
    PhysicsBox(glm::vec2 position, float width, float height) : Box(position, width, height) {
        Box::identifier = Objects::PhysicsBox;
    }
};
