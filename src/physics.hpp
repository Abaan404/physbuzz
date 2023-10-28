#pragma once

#include "physics/physics_box.hpp"
#include "store.hpp"

class PhysicsContext {
  public:
    PhysicsContext(GameObjectStore *storage);

    void tick();

  private:
    GameObjectStore *storage;
};
