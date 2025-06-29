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
    Resource<Model> model;
    Resource<ShaderPipeline> pipeline;
};

class Renderer : public System<RenderComponent> {
  public:
    struct Info {
        glm::ivec2 resolution;
        std::vector<Resource<ShaderPipeline>> postProcessing;
    };

    Renderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void resize(const glm::ivec2 &resolution);

    void tick(Scene &scene) const;
    void render(Scene &scene, ObjectID id) const;

    void target(const Framebuffer *framebuffer);

    const Framebuffer &getFramebuffer() const;

  private:
    Info m_Info;

    Framebuffer m_Framebuffer;
    const Framebuffer *m_TargetBuffer = nullptr;
};

} // namespace Physbuzz
