#pragma once

#include "../geometry/object.hpp"
#include "../geometry/physics_box.hpp"
#include "../geometry/physics_circle.hpp"
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdexcept>

class Collision {
  public:
    static bool collides(std::shared_ptr<GameObject> object1, std::shared_ptr<GameObject> object2);

  private:
    static bool check_collision_box_box(PhysicsBox &box1, PhysicsBox &box2);
    static bool check_collision_circle_circle(PhysicsCircle &circle1, PhysicsCircle &circle2);
    static bool check_collision_box_circle(PhysicsBox &box, PhysicsCircle &circle);
};
