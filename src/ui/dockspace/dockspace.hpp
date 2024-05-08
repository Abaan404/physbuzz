#pragma once

#include "../ui.hpp"

class Dockspace : public IUserInterface {
  public:
    void draw(Physbuzz::Renderer &renderer) override;
};
