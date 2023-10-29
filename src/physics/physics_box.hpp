#pragma once

#include "../geometry/box.hpp"

class PhysicsCircle;

class PhysicsBox : public Box {
  public:
    PhysicsBox(int x, int y, int width, int height, Mask mask) : Box(x, y, width, height, mask) {
        Box::identifier = Objects::PhysicsBox;
    };
    bool collides(GameObject *object) override;

    bool collides(PhysicsBox *box);
    bool collides(PhysicsCircle *circle);
};
