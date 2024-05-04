#pragma once

#include "physbuzz/object.hpp"
#include "physbuzz/scene.hpp"

class Collision {
  public:
    void tick(Physbuzz::Scene &scene);

  private:

    bool check(Physbuzz::Object &object1, Physbuzz::Object &object2);
    void resolve(Physbuzz::Object &object1, Physbuzz::Object &object2);
};

