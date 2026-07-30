#pragma once

#include "../ecs/system.hpp"
#include "../window/window.hpp"
#include "batch.hpp"
#include "compute/culling.hpp"
#include "defines.hpp"

namespace Physbuzz {

class GraphicsPipeline;
class DescriptorLayout;

namespace Builtin {

namespace PipelineForward {

struct PushConstants {
    std::uint32_t directionalCount;
    std::uint32_t spotCount;
    std::uint32_t pointCount;

    std::uint64_t materialBaseAddress;
};

struct Specialization {
    std::uint32_t enableShadows;
};

inline Resource<DescriptorLayout> ResourceLayoutGlobal = {"builtin/forward/global"};
inline Resource<DescriptorLayout> ResourceLayoutFrame = {"builtin/forward/frame"};

inline Resource<GraphicsPipeline> Resource = {"builtin/forward"};

bool build();

} // namespace PipelineForward

} // namespace Builtin

class ForwardRenderer : public System<RenderComponent> {
  public:
    constexpr static RenderNodeID Output = "builtin/forward";

    struct Info {
        ObjectID camera;
        std::shared_ptr<Window> window;

        struct {
            Resource<Texture> irradianceMap = {""};
        } resources;
    };

    ForwardRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    bool specialize(const Builtin::PipelineForward::Specialization &specialization);

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    BatchGenerator m_Batch = {{
        .resourceIdPrefix = std::format("{}/batch/", Output),
    }};

    FrustumCulling m_Culling = {{
        .batch = m_Batch,
        .objects = {
            .cameras = {m_Info.camera},
        },
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
