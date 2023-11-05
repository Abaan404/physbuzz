#pragma once

#include "../geometry/object.hpp"
#include "../geometry/physics_box.hpp"
#include "../geometry/physics_circle.hpp"

class Collision {
  public:
    static bool check_collision(GameObject *object1, GameObject *object2);

  private:
    static bool check_collision_box_box(PhysicsBox *object1, PhysicsBox *object2);
    static bool check_collision_circle_circle(PhysicsCircle *object1, PhysicsCircle *object2);
    static bool check_collision_box_circle(PhysicsBox *object1, PhysicsCircle *object2);
};
