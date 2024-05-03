#pragma once

#include <physbuzz/renderer.hpp>

class IUserInterface {
  public:
    bool show = true;
    virtual void draw(Physbuzz::Renderer &renderer) = 0;
};
