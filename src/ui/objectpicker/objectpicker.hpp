#pragma once

#include <glm/glm.hpp>
#include "../../renderer/framebuffer.hpp"
#include "../ui.hpp"

class ObjectPicker : public IUserInterface {
  public:
    void draw(Renderer &renderer) override;

  private:
    glm::ivec2 preview_size = glm::ivec2(120, 120);
    Framebuffer framebuffer = Framebuffer(preview_size);
};
