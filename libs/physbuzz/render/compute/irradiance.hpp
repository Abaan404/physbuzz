#pragma once

#include "../../graphics/layout.hpp"
#include "../../graphics/rendergraph.hpp"

namespace Physbuzz {

namespace Builtin {

namespace PipelineIrradiance {

inline Resource<DescriptorLayout> ResourceLayout = {"builtin/irradiance"};

inline Resource<ComputePipeline> Resource = {"builtin/irradiance"};

bool build();

} // namespace PipelineIrradiance

} // namespace Builtin

class IrradianceCompute {
  public:
    constexpr static RenderNodeID Output = "builtin/irradiance";

    struct Info {
        Resource<Texture> environmentMap = {""};
    };

    IrradianceCompute(const Info &info);

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;
    const Resource<Texture> &getIrradianceMap() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    Resource<Texture> m_Resource = std::format("{}/irradiance", m_Info.environmentMap);
};

} // namespace Physbuzz
