#pragma once

#include "../ecs/system.hpp"
#include "../graphics/pipeline.hpp"
#include "batch.hpp"
#include "compute/culling.hpp"
#include "defines.hpp"

namespace Physbuzz {

namespace Builtin {

namespace PipelineShadow {

struct Vertex {
    glm::vec3 position;

    static VertexDescription Description;
};

namespace Directional {

struct PushConstants {
    std::uint32_t objectCount;
    std::uint32_t cameraId;
};

inline Resource<Attachment> ResourceAttachment = {"builtin/shadow/directional"};

inline Resource<DescriptorLayout> ResourceLayoutFrame = {"builtin/shadow/directional/frame"};

inline Resource<GraphicsPipeline> Resource = {"builtin/shadow/directional"};

bool build(const glm::uvec2 &resolution);

} // namespace Directional

namespace Point {

struct PushConstants {
    std::uint32_t objectCount;
    std::uint32_t cameraId;
    std::uint32_t faceId;
};

inline Resource<Attachment> ResourceAttachment = {"builtin/shadow/point"};

inline Resource<DescriptorLayout> ResourceLayoutFrame = {"builtin/shadow/point/frame"};

inline Resource<GraphicsPipeline> Resource = {"builtin/shadow/point"};

bool build(const glm::uvec2 &resolution);

} // namespace Point

} // namespace PipelineShadow

} // namespace Builtin

struct ShadowComponent {};

class ShadowRenderer : public System<RenderComponent, ShadowComponent> {
  public:
    inline static RenderNodeID OutputDirectional = "builtin/shadow/directional";
    constexpr static RenderNodeID OutputPoint = "builtin/shadow/point";

    struct Info {
        glm::uvec2 resolution = {1024, 1024};
    };

    ShadowRenderer(const Info &info);

    bool build();
    bool destroy();

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    BatchGenerator m_Batch = {{
        .resourceIdPrefix = "builtin/shadow/",
    }};

    FrustumCulling m_Culling = {{
        .batch = m_Batch,
        .nodeIdPrefix = "builtin/shadow/culling/",
        .resourceIdPrefix = "builtin/shadow/culling/",
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
};

} // namespace Physbuzz
