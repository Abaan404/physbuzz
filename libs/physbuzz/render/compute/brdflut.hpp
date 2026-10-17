#pragma once

#include "../../graphics/layout.hpp"
#include "../../graphics/rendergraph.hpp"

namespace Physbuzz {

class BrdfLutCompute {
  public:
    inline static RenderNodeID Output = "builtin/brdf_lut";

    struct Resources {
        Resource<DescriptorLayout> layout;
        Resource<ComputePipeline> pipeline;
    };

    BrdfLutCompute();

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;
    const Resource<Texture> &getLut() const;

  private:
    Resources m_Resources;

    Resource<Texture> m_Lut;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    bool buildResources();
};

} // namespace Physbuzz
