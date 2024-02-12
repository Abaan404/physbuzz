#pragma once

#include "collision/collision.hpp"

#include <memory>
#include <vector>

class SceneManager {
  public:
    SceneManager(std::vector<std::shared_ptr<GameObject>> &objects) : objects(objects){};

    void tick();

  private:
    Collision collision;
    std::vector<std::shared_ptr<GameObject>> &objects;
};
