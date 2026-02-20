#pragma once

#include "../ecs/system.hpp"
#include "../graphics/pipeline.hpp"
#include "../graphics/rendergraph.hpp"
#include "../resources/resource.hpp"
#include "../window/window.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace RenderPipelineDeferred {

namespace Geometry {

struct PushConstants {
    std::uint64_t materialBaseAddress;
};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/deferred/geometry/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/deferred/geometry"};

bool build();

} // namespace Geometry

namespace Lighting {

struct PushConstants {
    std::uint32_t directionalCount;
    std::uint32_t spotCount;
    std::uint32_t pointCount;
};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/deferred/lighting/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/deferred/lighting"};

bool build();

} // namespace Lighting

inline std::array<Resource<Attachment>, 3> ResourceGBuffers = {
    Resource<Attachment>{"builtin/deferred/gBuffer0"},
    Resource<Attachment>{"builtin/deferred/gBuffer1"},
    Resource<Attachment>{"builtin/deferred/gBuffer2"},
};

bool build();

} // namespace RenderPipelineDeferred

} // namespace Builtin

class DeferredRenderer : public System<RenderComponent> {
  public:
    constexpr static RenderNodeID Output = "builtin/deferred";

    struct Info {
        ObjectID camera;
        std::shared_ptr<Window> window;
    };

    DeferredRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    std::vector<std::pair<Resource<Mesh>, std::size_t>> m_Batches;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
