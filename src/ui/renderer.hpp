#pragma once

#include "ui.hpp"

class Renderer : public IUserInterface {
  public:
    Renderer(Physbuzz::Scene *scene);

    void draw() override;
};
