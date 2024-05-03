#pragma once

#include "../ui.hpp"
#include <imgui.h>

class Dockspace : public IUserInterface {
  public:
    void draw(Physbuzz::Renderer &renderer) override;
};
