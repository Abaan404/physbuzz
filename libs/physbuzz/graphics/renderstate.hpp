#pragma once

#include "../ecs/defines.hpp"
#include "../resources/resource.hpp"
#include "mesh.hpp"
#include "rendergraph.hpp"

namespace Physbuzz {

class Scene;
class DynamicBuffer;

class RenderState {
  public:
    struct Info {
        ResourceID instanceBufferId;
        ResourceID indirectBufferId;
    };

    RenderState(const Info &info);

    bool build(const std::unordered_set<ObjectID> &objects);
    bool destroy();

    void draw(const RenderContext &context);

    const RenderNode &getRenderNode() const;
    const Info &getInfo() const;

  private:
    struct InstanceData {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 normal;
        std::uint32_t materialIdx;
    };

    Info m_Info;

    RenderNode m_RenderNode;

    std::vector<vk::DrawIndexedIndirectCommand> m_IndirectBuffer;
    std::vector<InstanceData> m_InstanceBuffer;

    std::vector<std::tuple<Resource<Mesh>, std::size_t>> m_IndirectMeshes;

    Resource<DynamicBuffer> m_Instance;
    Resource<DynamicBuffer> m_Indirect;
};

} // namespace Physbuzz
