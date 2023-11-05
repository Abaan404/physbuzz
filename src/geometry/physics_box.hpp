#pragma once

#include "box.hpp"

class PhysicsBox : public Box {
  public:
    PhysicsBox(float x, float y, float width, float height, SDL_Color color) : Box(x, y, width, height, color) {
        Box::identifier = Objects::PhysicsBox;
    }
};
