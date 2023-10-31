#pragma once

#include "../geometry/box.hpp"

class PhysicsCircle;

class PhysicsBox : public Box {
  public:
    PhysicsBox(float x, float y, float width, float height, Color color) : Box(x, y, width, height, color) {
        Box::identifier = Objects::PhysicsBox;
    };
    bool collides(GameObject *object) override;

    bool collides(PhysicsBox *box);
    bool collides(PhysicsCircle *circle);
};
