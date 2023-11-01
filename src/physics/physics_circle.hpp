#pragma once

#include "../geometry/circle.hpp"

class PhysicsBox;

class PhysicsCircle : public Circle {
  public:
    PhysicsCircle(float x, float y, float radius, SDL_Color color) : Circle(x, y, radius, color) {
        Circle::identifier = Objects::PhysicsCircle;
    };
    void collides(GameObject *object) override;

    void resolve_collision(PhysicsBox *box);
    void resolve_collision(PhysicsCircle *circle);

    bool check_collision(PhysicsCircle *circle);
    bool check_collision(PhysicsBox *box);
};
