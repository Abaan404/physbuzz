#pragma once

#include "../../ecs/defines.hpp"
#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

class Scene;
class DynamicBuffer;
class DescriptorLayout;
class Mesh;
class CameraFrustum;

class FrustumCulling {
  public:
    struct Info {
        Resource<DynamicBuffer> instance;
        Resource<DynamicBuffer> indirect;
        RenderNodeID nodeIdPrefix;
        ResourceID resourceIdPrefix;
    };

    FrustumCulling(const Info &info);

    bool build();
    bool destroy();

    void setCamera(const std::vector<ObjectID> &cameras, const std::vector<CameraFrustum> &frustums);

    const Resource<DynamicBuffer> &getCullingBuffer() const;

    const RenderGraph &getGraph() const;
    const Info &getInfo() const;

  private:
    struct CullingBufferData {
        std::uint32_t instanceOffset;
    };

    struct CameraBufferData {
        std::array<glm::vec4, 6> planes;
    };

    struct PushConstants {
        std::uint32_t objectCount;
    };

    Info m_Info;

    std::vector<ObjectID> m_Cameras;
    std::vector<CameraFrustum> m_Frustums;

    RenderGraph m_Graph = {{}};

    Resource<DynamicBuffer> m_CameraBuffer;
    Resource<DynamicBuffer> m_CullingBuffer;

    Resource<ComputePipeline> m_Pipeline;
    Resource<DescriptorLayout> m_Layout;
};

} // namespace Physbuzz
