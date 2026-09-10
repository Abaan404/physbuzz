#pragma once

#include "../graphics/rendergraph.hpp"
#include "../resources/resource.hpp"

namespace Physbuzz {

class Model;
class Transform;
class DynamicBuffer;
class DescriptorLayout;
class Mesh;

struct Renderable {
    Model *model;
    Transform *transform;
};

class RenderBatch {
  public:
    struct InstanceData {
        struct Bounding {
            alignas(16) glm::vec3 min;
            alignas(16) glm::vec3 max;
        };

        glm::mat4 model;
        glm::mat4 normal;
        std::uint32_t materialIdx;
        Bounding bounding;
    };

    struct Info {
        ResourceID resourceIdPrefix;
    };

    RenderBatch(const Info &info);

    bool build(const std::vector<Renderable> &renderables);
    bool destroy();

    void draw(const RenderContext &context) const;
    void draw(const RenderContext &context, const Resource<DynamicBuffer> &indirect, std::uint64_t offset = 0) const;

    const Resource<DynamicBuffer> &getIndirectBuffer() const;
    const Resource<DynamicBuffer> &getInstanceBuffer() const;

    const std::vector<std::size_t> &getInstanceObjects() const;
    const std::vector<InstanceData> &getInstanceData() const;

    std::size_t getObjectCount(std::uint32_t frameInFlight) const;
    std::size_t getDrawCount(std::uint32_t frameInFlight) const;

    const RenderNode &getRenderNode() const;
    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderNode m_RenderNode;

    std::vector<InstanceData> m_InstanceBuffer;
    std::vector<std::size_t> m_InstanceObjects;

    std::vector<vk::DrawIndexedIndirectCommand> m_IndirectBuffer;
    std::vector<std::tuple<Resource<Mesh>, std::size_t>> m_IndirectMeshes;

    Resource<DynamicBuffer> m_Instance;
    Resource<DynamicBuffer> m_Indirect;
};

} // namespace Physbuzz
