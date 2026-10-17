#pragma once

#include "../../graphics/layout.hpp"
#include "../../graphics/rendergraph.hpp"

namespace Physbuzz {

class PrefilterCompute {
  public:
    inline static RenderNodeID Output = "builtin/prefilter";

    struct Info {
        Resource<Texture> environmentMap;
    };

    struct Resources {
        struct PushConstants {
            std::uint32_t mipLevel;
            float roughness;
        };

        struct Specialization {
            std::uint32_t isCubemap;
        };

        Resource<DescriptorLayout> layout;
        Resource<ComputePipeline> pipeline;
    };

    PrefilterCompute(const Info &info);

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;
    const Resource<Texture> &getPrefilterMap() const;

    const Info &getInfo() const;

  private:
    Info m_Info;
    Resources m_Resources;

    Resource<Texture> m_PrefilterMap;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    bool buildResources(const std::uint32_t &maxMipLevels);
};

} // namespace Physbuzz
