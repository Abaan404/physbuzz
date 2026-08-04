#pragma once

#include "../../graphics/layout.hpp"
#include "../../graphics/rendergraph.hpp"

namespace Physbuzz {

namespace Builtin {

namespace PipelinePrefilter {

struct PushConstants {
    std::uint32_t mipLevel;
    float roughness;
};

struct Specialization {
    std::uint32_t isCubemap;
};

inline Resource<DescriptorLayout> ResourceLayout = {"builtin/prefilter"};

inline Resource<ComputePipeline> Resource = {"builtin/prefilter"};

bool build();

} // namespace PipelinePrefilter

} // namespace Builtin

class PrefilterCompute {
  public:
    constexpr static RenderNodeID Output = "builtin/prefilter";

    struct Info {
        Resource<Texture> environmentMap = {""};
    };

    PrefilterCompute(const Info &info);

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;
    const Resource<Texture> &getPrefilterMap() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    Resource<Texture> m_Resource = std::format("{}/prefilter", m_Info.environmentMap);
};

} // namespace Physbuzz
