#pragma once

#include "../ecs/system.hpp"
#include "../graphics/pipeline.hpp"
#include "../graphics/renderstate.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace RenderPipelineShadow {

namespace Directional {

inline Resource<Attachment> ResourceAttachment = {"builtin/shadow/directional"};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/shadow/directional/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/shadow/directional"};

bool build(const glm::uvec2 &resolution);

} // namespace Directional

namespace Point {

inline Resource<Attachment> ResourceAttachment = {"builtin/shadow/point"};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/shadow/point/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/shadow/point"};

bool build(const glm::uvec2 &resolution);

} // namespace Point

} // namespace RenderPipelineShadow

} // namespace Builtin

struct ShadowComponent {};

class ShadowRenderer : public System<RenderComponent, ShadowComponent> {
  public:
    constexpr static RenderNodeID Output2D = "builtin/shadow/2D";
    constexpr static RenderNodeID OutputCube = "builtin/shadow/cube";

    struct Info {
        glm::uvec2 resolution = {1024, 1024};
    };

    ShadowRenderer(const Info &info);

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderState m_State = {{
        .instanceBufferId = "builtin/shadow/instance",
        .indirectBufferId = "builtin/shadow/indirect",
    }};

    RenderGraph m_Graph = {{}};
};

} // namespace Physbuzz
