#pragma once

#include "physics/physics_box.hpp"
#include <vector>

class PhysicsContext {
  public:
    PhysicsContext(std::vector<GameObject *> *objects) : objects(objects){};

    void tick();

  private:
    std::vector<GameObject *> *objects;
};
