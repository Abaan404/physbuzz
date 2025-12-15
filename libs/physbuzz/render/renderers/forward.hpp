#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/resources.hpp"
#include "../shaders.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ShaderForward {

inline Resource<RenderPipeline> Resource = {"builtin/forward"};

bool build();

} // namespace ShaderForward

} // namespace Builtin

struct ForwardRenderComponent {
    Resource<RenderPipeline> pipeline = Builtin::ShaderForward::Resource;
};

class ForwardRenderer : public IRenderPass,
                        public System<RenderComponent, ForwardRenderComponent> {
  public:
    ForwardRenderer();

    bool build() override;
    bool destroy() override;

    void render(const vk::CommandBuffer &commandBuffer, std::uint32_t frameInFlight) override;
};

} // namespace Physbuzz
