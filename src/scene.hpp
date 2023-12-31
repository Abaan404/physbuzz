#pragma once

#include "collision/collision.hpp"
#include "geometry/object.hpp"
#include "dynamics/dynamics.hpp"

#include <cstdio>
#include <memory>
#include <vector>

class SceneManager {
  public:
    SceneManager(std::vector<std::shared_ptr<GameObject>> &objects) : objects(objects){};

    void tick();

  private:
    Collision collision;
    Dynamics dynamics;
    std::vector<std::shared_ptr<GameObject>> &objects;
};
