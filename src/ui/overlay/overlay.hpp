#pragma once

#include "../ui.hpp"

class FrametimeOverlay : public IUserInterface {
  public:
    FrametimeOverlay(Physbuzz::Scene *scene);

    void draw() override;
};
