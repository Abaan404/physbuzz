#pragma once

#include "collision/collision.hpp"
#include "geometry/object.hpp"

#include <cstdio>
#include <memory>
#include <vector>

class PhysicsContext {
  public:
    PhysicsContext(std::vector<std::shared_ptr<GameObject>> &objects) : objects(objects){};

    void tick();

  private:
    Collision collision;
    std::vector<std::shared_ptr<GameObject>> &objects;
};
