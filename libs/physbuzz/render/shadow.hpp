#pragma once

#include "../ecs/system.hpp"
#include "../graphics/pipeline.hpp"
#include "defines.hpp"
#include "physbuzz/graphics/rendergraph.hpp"

namespace Physbuzz {

namespace Builtin {

namespace RenderPipelineShadowDirectional {

inline Resource<DynamicBuffer> ResourceModel = {"builtin/shadow/directional/model"};
inline Resource<Attachment> ResourceAttachment = {"builtin/shadow/directional"};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/shadow/directional/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/shadow/directional"};

bool build(const glm::uvec2 &resolution);

} // namespace RenderPipelineShadowDirectional

namespace RenderPipelineShadowPoint {

struct PushConstants {
};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/shadow/point/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/shadow/point"};

bool build();

} // namespace RenderPipelineShadowPoint

} // namespace Builtin

struct ShadowComponent {};

class ShadowRenderer : public System<RenderComponent, ShadowComponent> {
  public:
    constexpr static RenderNodeID Output = "builtin/shadow";

    struct Info {
        glm::uvec2 resolution = {1024, 1024};
    };

    ShadowRenderer(const Info &info);

    bool build();
    bool destroy();

    void resize(const glm::ivec2 &resolution);

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    void tickPoint() const;

    Info m_Info;

    std::vector<std::tuple<Resource<Mesh>, std::size_t>> m_Batches;

    RenderGraph m_Graph = {{
        .output = Output,
    }};
};

} // namespace Physbuzz
