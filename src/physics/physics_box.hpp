#pragma once

#include "../geometry/box.hpp"

class PhysicsCircle;

class PhysicsBox : public Box {
  public:
    PhysicsBox(float x, float y, float width, float height, SDL_Color color) : Box(x, y, width, height, color) {
        Box::identifier = Objects::PhysicsBox;
    };
    void collides(GameObject *object) override;

    void resolve_collision(PhysicsBox *box);
    void resolve_collision(PhysicsCircle *circle);

    bool check_collision(PhysicsBox *box);
    bool check_collision(PhysicsCircle *circle);
};
