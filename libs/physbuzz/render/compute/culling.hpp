#pragma once

#include "../../ecs/defines.hpp"
#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

class Scene;
class DynamicBuffer;
class DescriptorLayout;
class Mesh;
class Frustum;
class BatchGenerator;

class FrustumCulling {
  public:
    struct Objects {
        std::unordered_set<ObjectID> cameras;
        std::unordered_set<ObjectID> directionalLights;
        std::unordered_set<ObjectID> pointLights;
        std::unordered_set<ObjectID> spotLights;
    };

    struct Info {
        const BatchGenerator &batch;
        Objects objects;
        RenderNodeID nodeIdPrefix;
        ResourceID resourceIdPrefix;
    };

    FrustumCulling(const Info &info);

    bool build();
    bool destroy();

    void setObjects(const Objects &objects);

    const Resource<DynamicBuffer> &getVisibleInstanceBuffer() const;
    const Resource<DynamicBuffer> &getIndirectBuffer() const;

    std::uint32_t getFrustumId(ObjectID object) const;
    std::uint64_t getIndirectOffset(std::uint32_t frustumId, std::uint32_t frameInFlight) const;

    const RenderGraph &getGraph() const;
    const Info &getInfo() const;

  private:
    struct CullingBufferData {
        std::uint32_t instanceOffset;
    };

    struct FrustumBufferData {
        std::array<glm::vec4, 6> planes;
    };

    struct PushConstants {
        std::uint32_t objectCount;
        std::uint32_t drawCount;
    };

    Info m_Info;

    RenderGraph m_Graph = {{}};

    std::vector<FrustumBufferData> m_FrustumBuffer;
    std::unordered_map<ObjectID, std::uint64_t> m_FrustumBufferIds;

    Resource<DynamicBuffer> m_Frustum;
    Resource<DynamicBuffer> m_VisibleInstance;
    Resource<DynamicBuffer> m_Indirect;

    Resource<ComputePipeline> m_Pipeline;
    Resource<DescriptorLayout> m_Layout;
};

} // namespace Physbuzz
