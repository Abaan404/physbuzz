#pragma once

#include "geometry/object.hpp"

#include <cstdio>
#include <memory>
#include <vector>

class PhysicsContext {
  public:
    PhysicsContext(std::vector<std::shared_ptr<GameObject>> &objects) : objects(objects){};

    void tick();

  private:
    std::vector<std::shared_ptr<GameObject>> &objects;
};
