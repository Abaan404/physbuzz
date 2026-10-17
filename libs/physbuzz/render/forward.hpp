#pragma once

#include "../ecs/system.hpp"
#include "../graphics/material.hpp"
#include "../window/window.hpp"
#include "batch.hpp"
#include "compute/brdflut.hpp"
#include "compute/culling.hpp"
#include "compute/irradiance.hpp"
#include "compute/prefilter.hpp"
#include "defines.hpp"

namespace Physbuzz {

class ForwardRenderer : public System<RenderComponent, TransformComponent> {
  public:
    inline static RenderNodeID Output = "builtin/forward";

    struct Info {
        std::shared_ptr<Window> window;

        Resource<Texture> environmentMap;
    };

    struct Resources {
        struct PushConstants {
            std::uint32_t directionalCount;
            std::uint32_t spotCount;
            std::uint32_t pointCount;

            std::uint64_t materialBaseAddress;
        };

        struct Specialization {
            std::uint32_t enableShadows;
        };

        Resource<DescriptorLayout> layoutGlobal;
        Resource<DescriptorLayout> layoutFrame;
        Resource<GraphicsPipeline> pipeline;
    };

    ForwardRenderer(Scene *scene, const Info &info);

    bool build() override;
    bool destroy() override;

    void tick();

    bool specialize(const Resources::Specialization &specialization);

    const RenderBatch &getBatch() const;
    const RenderGraph &getGraph() const;
    const Resources &getResources() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    Resources m_Resources;
    std::vector<Renderable> m_Renderables;

    RenderBatch m_Batch = {{
        .resourceNamePrefix = std::format("{}/batch/", Output),
    }};

    FrustumCulling m_Culling = {{
        .batch = m_Batch,
        .nodeIdPrefix = std::format("{}/culling/", Output),
        .resourceNamePrefix = std::format("{}/culling/", Output),
    }};

    IrradianceCompute m_Irradiance = {{
        .environmentMap = m_Info.environmentMap,
    }};

    PrefilterCompute m_Prefilter = {{
        .environmentMap = m_Info.environmentMap,
    }};

    BrdfLutCompute m_BrdfLut;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        EventID resize = -1;
    } m_Events = {};

    bool buildResources(const MaterialAllocator::Resources &material);
};

} // namespace Physbuzz
