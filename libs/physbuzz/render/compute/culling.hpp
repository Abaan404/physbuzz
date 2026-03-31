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
        Resource<DynamicBuffer> instance;
        Resource<DynamicBuffer> indirect;
        RenderNodeID nodeIdPrefix;
        ResourceID resourceIdPrefix;
    };

    FrustumCulling(const Info &info);

    bool build();
    bool destroy();

    const Resource<DynamicBuffer> getBuffer() const;

    const RenderNode &getRenderNode() const;
    const Info &getInfo() const;

  private:
    struct BufferData {
        std::uint32_t instanceOffset;
    };

    struct PushConstants {
        std::array<glm::vec4, 6> planes;
    };

    Info m_Info;

    RenderNode m_RenderNode;

    Resource<DynamicBuffer> m_Buffer;

    Resource<ComputePipeline> m_Pipeline;
    Resource<DescriptorLayout> m_Layout;
};

} // namespace Physbuzz
