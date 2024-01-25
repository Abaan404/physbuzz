#pragma once

#include "../geometry/object.hpp"
#include "../geometry/physics_box.hpp"
#include "../geometry/physics_circle.hpp"
#include <memory>

class Collision {
  public:
    static bool tick(std::shared_ptr<GameObject> object1, std::shared_ptr<GameObject> object2);

  private:
    static bool check_collision(PhysicsBox &box1, PhysicsBox &box2);
    static bool check_collision(PhysicsCircle &circle1, PhysicsCircle &circle2);
    static bool check_collision(PhysicsBox &box, PhysicsCircle &circle);

    static void resolve_collision(PhysicsBox &box1, PhysicsBox &box2);
    static void resolve_collision(PhysicsCircle &circle1, PhysicsCircle &circle2);
    static void resolve_collision(PhysicsBox &box, PhysicsCircle &circle);
};
