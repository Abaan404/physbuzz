#pragma once

#include "../ui.hpp"

class Demo : public IUserInterface {
  public:
    void draw(Physbuzz::Renderer &renderer) override;
};
