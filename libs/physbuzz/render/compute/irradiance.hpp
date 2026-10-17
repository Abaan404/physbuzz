#pragma once

#include "../../graphics/layout.hpp"
#include "../../graphics/rendergraph.hpp"

namespace Physbuzz {

class IrradianceCompute {
  public:
    inline static RenderNodeID Output = "builtin/irradiance";

    struct Info {
        Resource<Texture> environmentMap;
    };

    struct Resources {
        struct Specialization {
            std::uint32_t isCubemap;
        };

        Resource<DescriptorLayout> layout;
        Resource<ComputePipeline> pipeline;
    };

    IrradianceCompute(const Info &info);

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;
    const Resource<Texture> &getIrradianceMap() const;

    const Info &getInfo() const;

  private:
    Info m_Info;
    Resources m_Resources;

    Resource<Texture> m_IrradianceMap;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    bool buildResources();
};

} // namespace Physbuzz
