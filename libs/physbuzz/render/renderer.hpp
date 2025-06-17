#pragma once

#include "../ecs/system.hpp"
#include "../resources/handle.hpp"
#include "framebuffer.hpp"
#include "model.hpp"
#include "shaders.hpp"
#include "transform.hpp"

namespace Physbuzz {

class Framebuffer;

inline ResourceHandle<ModelResource> ScreenQuad = {"builtin/renderer/screenquad"};

struct RenderComponent {
    Transform transform;
    ResourceHandle<ModelResource> model;
    ResourceHandle<ShaderPipelineResource> pipeline;
};

struct RendererInfo {
    FramebufferInfo framebuffer;
    std::vector<ResourceHandle<ShaderPipelineResource>> postProcessing;
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
