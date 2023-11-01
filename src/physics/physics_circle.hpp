#pragma once

#include "../geometry/circle.hpp"

class PhysicsBox;

class PhysicsCircle : public Circle {
  public:
    PhysicsCircle(float x, float y, float radius, SDL_Color color) : Circle(x, y, radius, color) {
        Circle::identifier = Objects::PhysicsCircle;
    };
    bool collides(GameObject *object) override;

    bool collides(PhysicsCircle *circle);
    bool collides(PhysicsBox *box);
};
