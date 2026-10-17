#pragma once

#include "../ecs/system.hpp"
#include "../graphics/material.hpp"
#include "../graphics/pipeline.hpp"
#include "../resources/registry.hpp"
#include "../window/window.hpp"
#include "batch.hpp"
#include "compute/culling.hpp"
#include "defines.hpp"
#include "resources.hpp"
#include <format>

namespace Physbuzz {

class DeferredRenderer : public System<RenderComponent, TransformComponent> {
  public:
    constexpr static RenderNodeID Output = "builtin/deferred";

    struct Info {
        std::shared_ptr<Window> window;
    };

    struct Resources {
        std::array<Resource<Attachment>, 3> gbuffers;

        struct Geometry {
            struct PushConstants {
                std::uint64_t materialBaseAddress;
            };

            Resource<DescriptorLayout> layout;
            Resource<GraphicsPipeline> pipeline;
        } geometry;

        struct Lighting {
            struct PushConstants {
                std::uint32_t directionalCount;
                std::uint32_t spotCount;
                std::uint32_t pointCount;
            };

            struct Specialization {
                std::uint32_t enableShadows;
            };

            Resource<DescriptorLayout> layout;
            Resource<GraphicsPipeline> pipeline;
        } lighting;
    };

    DeferredRenderer(Scene *scene, const Info &info);

    bool build() override;
    bool destroy() override;

    void tick();

    bool specialize(const Resources::Lighting::Specialization &specialization);

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

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        EventID resize = -1;
    } m_Events = {};

    bool buildResources(const MaterialAllocator::Resources &material);

    static std::optional<Resources::Geometry> buildGeometryResources(const MaterialAllocator::Resources &material);
    static std::optional<Resources::Lighting> buildLightingResources();
};

} // namespace Physbuzz
