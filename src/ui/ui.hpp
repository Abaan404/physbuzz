#pragma once

#include "../renderer/renderer.hpp"

class IUserInterface {
  public:
    virtual ~IUserInterface() = default;

    bool show = false;
    virtual void draw(Renderer &renderer) = 0;
};
