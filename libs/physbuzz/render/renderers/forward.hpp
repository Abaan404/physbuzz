#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/resources.hpp"
#include "../buffer.hpp"
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

    void render(const RenderContext &context) override;

  private:
    void resize(const glm::uvec2 &resolution);

    glm::uvec2 m_Resolution;

    struct {
        Image image = {{
            .usage = Image::ImageUsageFlagBits::eDepthStencilAttachment,
            .type = Image::Type::e2D,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = Image::Format::eD32Sfloat,
        }};
        vk::ImageView view;
    } m_Depth;

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
