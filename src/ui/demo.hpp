#pragma once

#include "ui.hpp"

class Demo : public IUserInterface {
  public:
    Demo(Physbuzz::Scene *scene);

    void draw() override;
};
