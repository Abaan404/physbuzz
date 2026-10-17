#pragma once

#include "../ecs/system.hpp"
#include "../graphics/pipeline.hpp"
#include "../resources/registry.hpp"
#include "batch.hpp"
#include "compute/culling.hpp"
#include "defines.hpp"
#include "resources.hpp"
#include <optional>

namespace Physbuzz {

struct ShadowComponent {
    Model model;
};

class ShadowRenderer : public System<ShadowComponent, TransformComponent> {
  public:
    inline static RenderNodeID OutputDirectional = "builtin/shadow/directional";
    constexpr static RenderNodeID OutputPoint = "builtin/shadow/point";

    struct Vertex {
        glm::vec3 position;

        static VertexDescription Description;
    };

    struct Info {
        glm::uvec2 resolution = {1024, 1024};
    };

    struct Resources {
        struct Directional {
            struct PushConstants {
                std::uint32_t objectCount;
                std::uint32_t cameraId;
            };

            Resource<Attachment> attachment;
            Resource<DescriptorLayout> layout;
            Resource<GraphicsPipeline> pipeline;
        } directional;

        struct Point {
            struct PushConstants {
                std::uint32_t objectCount;
                std::uint32_t cameraId;
                std::uint32_t faceId;
            };

            Resource<Attachment> attachment;
            Resource<DescriptorLayout> layout;
            Resource<GraphicsPipeline> pipeline;
        } point;
    };

    ShadowRenderer(Scene *scene, const Info &info);

    bool build() override;
    bool destroy() override;

    void tick();

    const RenderBatch &getBatch() const;
    const RenderGraph &getGraph() const;
    const Resources &getResources() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    Resources m_Resources;
    std::vector<Renderable> m_Renderables;

    RenderBatch m_Batch = {{
        .resourceNamePrefix = "builtin/shadow/batch/",
    }};

    FrustumCulling m_Culling = {{
        .batch = m_Batch,
        .nodeIdPrefix = "builtin/shadow/culling/",
        .resourceNamePrefix = "builtin/shadow/culling/",
    }};

    RenderGraph m_Graph = {{}};

    struct {
        struct {
            EventID add = -1;
            EventID remove = -1;
        } directional;

        struct {
            EventID add = -1;
            EventID remove = -1;
        } point;

        struct {
            EventID add = -1;
            EventID remove = -1;
        } spot;
    } m_Events;

    bool buildResources();

    static std::optional<Resources::Directional> buildDirectionalResources(const glm::uvec2 &resolution);
    static std::optional<Resources::Point> buildPointResources(const glm::uvec2 &resolution);
};

} // namespace Physbuzz
