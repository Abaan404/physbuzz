#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/resources.hpp"
#include "../framebuffer.hpp"
#include "../shaders.hpp"
#include "defines.hpp"

namespace Physbuzz {

struct ForwardRenderComponent {
    Resource<ShaderPipeline> pipeline;
};

class ForwardRenderer : public IRenderer,
                        public System<RenderComponent, ForwardRenderComponent> {
  public:
    struct Info {
    };

    ForwardRenderer(const Info &info, const glm::ivec2 &resolution);

    bool build() override;
    bool destroy() override;

    void resize(const glm::ivec2 &resolution) override;

    void tick() const;
    void render(ObjectID id) const;

    const Framebuffer &getOutput() const override;
    const Info &getInfo() const;

  private:
    Info m_Info;

    Framebuffer m_Output;
};

} // namespace Physbuzz
