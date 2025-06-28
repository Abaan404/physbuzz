#pragma once

#include "../ecs/system.hpp"
#include "renderer.hpp"

namespace Physbuzz {

struct ShadowComponent {};

struct ShadowInfo {
    glm::ivec2 resolution = {1280, 720};
    float orthoSize = 100.0f;
    float depth = 100.0f;
};

class Shadow : public System<RenderComponent, ShadowComponent> {
  public:
    struct Framebuffers {
        Framebuffer directional;
        Framebuffer point;
    };

    Shadow(const ShadowInfo &info);

    bool build();
    bool destroy();

    void resize(const glm::ivec2 &resolution);

    void tick(Scene &scene) const;

    const Framebuffers &getFramebuffers() const;
    const ShadowInfo &getInfo() const;

  private:
    void tickDirectional(Scene &scene) const;
    void tickPoint(Scene &scene) const;

    ShadowInfo m_Info;
    Framebuffers m_Framebuffers;
};

} // namespace Physbuzz
