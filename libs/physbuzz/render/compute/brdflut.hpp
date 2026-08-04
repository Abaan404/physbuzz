#pragma once

#include "../../graphics/layout.hpp"
#include "../../graphics/rendergraph.hpp"

namespace Physbuzz {

namespace Builtin {

namespace PipelineBrdfLut {

inline Resource<DescriptorLayout> ResourceLayout = {"builtin/brdf_lut"};

inline Resource<Texture> ResourceLut = {"builtin/brdf_lut"};

inline Resource<ComputePipeline> Resource = {"builtin/brdf_lut"};

bool build();

} // namespace PipelineBrdfLut

} // namespace Builtin

class BrdfLutCompute {
  public:
    constexpr static RenderNodeID Output = "builtin/brdf_lut";

    BrdfLutCompute();

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;

  private:
    RenderGraph m_Graph = {{
        .output = Output,
    }};
};

} // namespace Physbuzz
