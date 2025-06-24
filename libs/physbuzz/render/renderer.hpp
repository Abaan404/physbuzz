#pragma once

#include "../ecs/system.hpp"
#include "framebuffer.hpp"
#include "model.hpp"
#include "shaders.hpp"
#include "transform.hpp"

namespace Physbuzz {

class Framebuffer;

struct RenderComponent {
    Transform transform;
    ResourceHandle<ModelResource> model;
    ResourceHandle<ShaderPipelineResource> pipeline;
};

struct RendererInfo {
    FramebufferInfo framebuffer;
    std::vector<ResourceHandle<ShaderPipelineResource>> postProcessing;
    int screenIndex = 0;
};

class Renderer : public System<RenderComponent> {
  public:
    Renderer(const RendererInfo &info);

    bool build() override;
    bool destroy() override;

    void resize(const glm::ivec2 &resolution);

    void tick(Scene &scene) const;
    void render(Scene &scene, ObjectID id) const;

    void target(const Framebuffer *framebuffer);

    const Framebuffer &getFramebuffer() const;

  private:
    RendererInfo m_Info;

    Framebuffer m_Framebuffer;
    const Framebuffer *m_TargetBuffer = nullptr;
};

} // namespace Physbuzz
