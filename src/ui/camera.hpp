#pragma once

#include "ui.hpp"

class Camera : public IUserInterface {
  public:
    Camera(Physbuzz::Scene *scene);

    void draw() override;
};

