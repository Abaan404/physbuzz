#pragma once

#include "../ui.hpp"

class ObjectList : public IUserInterface {
  public:
    void draw(Physbuzz::Renderer &renderer) override;
};
