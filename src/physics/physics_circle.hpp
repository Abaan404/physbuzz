#pragma once

#include "../geometry/circle.hpp"

class PhysicsBox;

class PhysicsCircle : public Circle {
  public:
    PhysicsCircle(int x, int y, int radius, Mask mask) : Circle(x, y, radius, mask) {
        Circle::identifier = Objects::PhysicsCircle;
    };
    bool collides(GameObject *object) override;

    bool collides(PhysicsCircle *circle);
    bool collides(PhysicsBox *box);
};
