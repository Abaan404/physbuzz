#pragma once

#include "ui.hpp"

class RenderGraph : public IUserInterface {
  public:
    RenderGraph(Physbuzz::Scene *scene);

    void draw() override;
};
