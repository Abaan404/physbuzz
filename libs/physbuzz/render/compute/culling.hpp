#pragma once

#include "../../ecs/defines.hpp"
#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

class Scene;
class DynamicBuffer;
class DescriptorLayout;
class Mesh;

class FrustumCulling {
  public:
    struct Info {
        ObjectID camera;
        Resource<DynamicBuffer> sceneBuffer;
        Resource<DynamicBuffer> indirectBuffer;
        RenderNodeID nodeIdPrefix;
        ResourceID resourceIdPrefix;
    };

    FrustumCulling(const Info &info);

    bool build();
    bool destroy();

    const Resource<DynamicBuffer> getInstanceBuffer() const;
    const Resource<DynamicBuffer> getIndirectBuffer() const;

    const RenderGraph &getGraph() const;
    const Info &getInfo() const;

  private:
    struct ObjectData {
        glm::mat4 model;
        glm::mat4 normal;
        alignas(16) std::uint32_t materialIdx;
    };

    struct StateData {
        std::uint32_t instanceOffset;
    };

    struct PushConstants {
        std::array<glm::vec4, 6> planes;
    };

    Info m_Info;

    RenderGraph m_RenderGraph;

    Resource<DynamicBuffer> m_Instance;
    Resource<DynamicBuffer> m_State;

    Resource<ComputePipeline> m_Pipeline;
    Resource<DescriptorLayout> m_Layout;
};

} // namespace Physbuzz
