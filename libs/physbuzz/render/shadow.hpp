#pragma once

#include "../ecs/system.hpp"
#include "renderer.hpp"

namespace Physbuzz {

struct ShadowInfo {
    glm::ivec2 resolution = {1000, 1000};
    float orthoSize = 100.0f;
    float depth = 100.0f;
};

class Shadow : public System<RenderComponent> {
  public:
    Shadow(const ShadowInfo &info);

    bool build();
    bool destroy();

    void tick(Scene &scene) const;

    const Framebuffer &getFramebuffer();

  private:
    ShadowInfo m_Info;
    Framebuffer m_Framebuffer;
};

} // namespace Physbuzz
