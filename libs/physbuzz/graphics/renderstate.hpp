#pragma once

#include "../ecs/defines.hpp"
#include "../resources/resource.hpp"
#include "rendergraph.hpp"

namespace Physbuzz {

class Scene;
class DynamicBuffer;
class DescriptorLayout;
class Mesh;

class RenderState {
  public:
    struct Info {
        RenderNodeID nodeIdPrefix;
        ResourceID resourceIdPrefix;
    };

    RenderState(const Info &info);

    bool build(const std::unordered_set<ObjectID> &objects);
    bool destroy();

    void draw(const RenderContext &context);

    const RenderGraph &getGraph() const;
    const Info &getInfo() const;

    const Resource<DynamicBuffer> getInstanceBuffer() const;
    const Resource<DynamicBuffer> getIndirectBuffer() const;

  private:
    struct PushConstants {
        std::uint32_t directionalCount;
        std::uint32_t spotCount;
        std::uint32_t pointCount;

        std::uint64_t materialBaseAddress;
    };

    struct InstanceData {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 normal;
        std::uint32_t materialIdx;
    };

    Info m_Info;

    RenderGraph m_RenderGraph;

    std::vector<vk::DrawIndexedIndirectCommand> m_IndirectBuffer;
    std::vector<InstanceData> m_InstanceBuffer;

    std::vector<std::tuple<Resource<Mesh>, std::size_t>> m_IndirectMeshes;

    Resource<DynamicBuffer> m_Instance;
    Resource<DynamicBuffer> m_Indirect;

    Resource<ComputePipeline> m_Pipeline;
    Resource<DescriptorLayout> m_Layout;
};

} // namespace Physbuzz
