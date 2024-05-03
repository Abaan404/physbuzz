#pragma once

#include "../ui.hpp"

class FrametimeOverlay : public IUserInterface {
  public:
    void draw(Physbuzz::Renderer &renderer) override;
};
