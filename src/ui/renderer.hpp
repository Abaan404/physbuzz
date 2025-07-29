#pragma once

#include "ui.hpp"
#include <physbuzz/render/framebuffer.hpp>

class Renderer : public IUserInterface {
  public:
    Renderer(Physbuzz::Scene *scene);

    void draw() override;
};
