#pragma once

#include "../geometry/object.hpp"
#include "../geometry/box/box.hpp"
#include "../geometry/circle/circle.hpp"
#include <memory>

class Collision {
  public:
    static bool tick(std::shared_ptr<GameObject> object1, std::shared_ptr<GameObject> object2);

  private:
    static bool check_collision(Box &box1, Box &box2);
    static bool check_collision(Circle &circle1, Circle &circle2);
    static bool check_collision(Box &box, Circle &circle);

    static void resolve_collision(Box &box1, Box &box2);
    static void resolve_collision(Circle &circle1, Circle &circle2);
    static void resolve_collision(Box &box, Circle &circle);
};
