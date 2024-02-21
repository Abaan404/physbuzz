#pragma once

#include "../ui.hpp"

class Viewport : public IUserInterface {
  public:
    Viewport(Renderer &renderer);

    void draw(Renderer &renderer) override;

  private:
    glm::ivec2 resolution = glm::ivec2(1280, 720);
    Framebuffer framebuffer = Framebuffer(resolution);
    Renderer &renderer;
};
