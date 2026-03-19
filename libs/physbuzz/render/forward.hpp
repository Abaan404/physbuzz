#pragma once

#include "../ecs/system.hpp"
#include "../graphics/renderstate.hpp"
#include "../window/window.hpp"
#include "defines.hpp"

namespace Physbuzz {

class RenderPipeline;
class PipelineLayout;

namespace Builtin {

namespace RenderPipelineForward {

struct PushConstants {
    std::uint32_t directionalCount;
    std::uint32_t spotCount;
    std::uint32_t pointCount;

    std::uint64_t materialBaseAddress;
};

struct Specialization {
    std::uint32_t enableShadows;
};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/forward/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/forward"};

bool build();

} // namespace RenderPipelineForward

} // namespace Builtin

class ForwardRenderer : public System<RenderComponent> {
  public:
    constexpr static RenderNodeID Output = "builtin/forward";

    struct Info {
        ObjectID camera;
        std::shared_ptr<Window> window;
    };

    ForwardRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    bool specialize(const Builtin::RenderPipelineForward::Specialization &specialization);

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderState m_State = {{
        .instanceBufferId = "builtin/forward/instance",
        .indirectBufferId = "builtin/forward/indirect",
    }};

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
