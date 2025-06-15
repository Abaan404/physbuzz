#pragma once

#include "framebuffer.hpp"
#include "model.hpp"
#include "shaders.hpp"
#include "transform.hpp"

namespace Physbuzz {

struct RenderComponent {
    Transform transform;
    ResourceHandle<ModelResource> model;
    std::vector<ResourceHandle<ShaderPipelineResource>> renderpasses;
};

class Framebuffer;

struct RendererInfo {
    FramebufferInfo framebuffer;
};

class Renderer : public System<RenderComponent> {
  public:
    Renderer(const RendererInfo &info);

    void build() override;
    void destroy() override;

    void resize(const glm::ivec2 &resolution);

    void tick(Scene &scene);
    void render(Scene &scene, ObjectID id);

    void target(const Framebuffer *framebuffer);

    const Framebuffer &getFramebuffer();

  private:
    RendererInfo m_Info;

    Framebuffer m_Framebuffer;
    const Framebuffer *m_TargetBuffer = nullptr;
};

} // namespace Physbuzz
