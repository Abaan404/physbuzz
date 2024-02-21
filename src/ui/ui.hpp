#pragma once

#include "../renderer/renderer.hpp"

class IUserInterface {
  public:
    bool show = true;
    virtual void draw(Renderer &renderer) = 0;
};
