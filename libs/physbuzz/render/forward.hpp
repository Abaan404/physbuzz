#pragma once

#include "../ecs/system.hpp"
#include "../graphics/rendergraph.hpp"
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

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/forward/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/forward"};

bool build();

} // namespace RenderPipelineForward

} // namespace Builtin

class ForwardRenderer : public System<RenderComponent> {
  public:
    struct Info {
        ObjectID camera;
        std::shared_ptr<Window> window;

        Resource<RenderPipeline> pipeline = Builtin::RenderPipelineForward::Resource;
    };

    ForwardRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderGraph m_Graph;
    std::vector<std::pair<Resource<Mesh>, std::size_t>> m_Batches;

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
