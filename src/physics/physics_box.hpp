#pragma once

#include "../geometry/box.hpp"

class PhysicsBox final : public PhysicsObject, public Box {
  public:
    PhysicsBox(int x, int y, int width, int height, Mask mask) : Box(x, y, width, height, mask) {};
    bool collides(PhysicsBox *box);
    bool collides(PhysicsObject *object) override;
};
