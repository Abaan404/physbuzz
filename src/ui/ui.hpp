#pragma once

#include <physbuzz/ecs/scene.hpp>

class IUserInterface {
  public:
    IUserInterface(Physbuzz::Scene *scene);
    virtual ~IUserInterface() = default;

    bool show = true;
    virtual void draw() = 0;

  protected:
    Physbuzz::Scene *m_Scene = nullptr;
};
