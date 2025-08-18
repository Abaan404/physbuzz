#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/resources.hpp"
#include "../framebuffer.hpp"
#include "../shaders.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ShaderForward {

inline Resource<ShaderPipeline> Resource = {"builtin/forward"};

bool build();

} // namespace ShaderForward

} // namespace Builtin

struct ForwardRenderComponent {
    Resource<ShaderPipeline> pipeline = Builtin::ShaderForward::Resource;
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

    void tick(const vk::CommandBuffer &commandBuffer);

    void render(const vk::CommandBuffer &commandBuffer, ObjectID id);

    const Framebuffer &getOutput() const override;
    const Info &getInfo() const;

  private:
    Info m_Info;

    Framebuffer m_Output;
};

} // namespace Physbuzz
