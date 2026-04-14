#pragma once

#include "../ecs/system.hpp"
#include "../graphics/pipeline.hpp"
#include "../resources/resource.hpp"
#include "../window/window.hpp"
#include "batch.hpp"
#include "compute/culling.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace PipelineDeferred {

namespace Geometry {

struct PushConstants {
    std::uint64_t materialBaseAddress;
};

inline Resource<DescriptorLayout> ResourceLayoutFrame = {"builtin/deferred/geometry/frame"};

inline Resource<GraphicsPipeline> Resource = {"builtin/deferred/geometry"};

bool build();

} // namespace Geometry

namespace Lighting {

struct PushConstants {
    std::uint32_t directionalCount;
    std::uint32_t spotCount;
    std::uint32_t pointCount;
};

struct Specialization {
    std::uint32_t enableShadows;
};

inline Resource<DescriptorLayout> ResourceLayoutFrame = {"builtin/deferred/lighting/frame"};

inline Resource<GraphicsPipeline> Resource = {"builtin/deferred/lighting"};

bool build();

} // namespace Lighting

inline std::array<Resource<Attachment>, 3> ResourceGBuffers = {
    Resource<Attachment>{"builtin/deferred/gBuffer0"},
    Resource<Attachment>{"builtin/deferred/gBuffer1"},
    Resource<Attachment>{"builtin/deferred/gBuffer2"},
};

bool build();

} // namespace PipelineDeferred

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

    bool specialize(const Builtin::PipelineDeferred::Lighting::Specialization &specialization);

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    BatchGenerator m_Batch = {{
        .resourceIdPrefix = std::format("{}/batch/", Output),
    }};

    FrustumCulling m_Culling = {{
        .instance = m_Batch.getInstanceBuffer(),
        .indirect = m_Batch.getIndirectBuffer(),
        .nodeIdPrefix = std::format("{}/culling/", Output),
        .resourceIdPrefix = std::format("{}/culling/", Output),
    }};

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
