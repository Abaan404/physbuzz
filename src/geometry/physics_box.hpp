#pragma once

#include "box.hpp"

class PhysicsBox : public DynamicObject, public Box {
  public:
    PhysicsBox(glm::vec2 position, float width, float height, SDL_Color color) : Box(position, width, height, color) {
        Box::identifier = Objects::PhysicsBox;
    }
};
