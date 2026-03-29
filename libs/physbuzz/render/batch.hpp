#pragma once

#include "../ecs/defines.hpp"
#include "../graphics/rendergraph.hpp"
#include "../resources/resource.hpp"

namespace Physbuzz {

class DynamicBuffer;
class DescriptorLayout;
class Mesh;

class BatchGenerator {
  public:
    struct SceneDataBuffer {
        struct Bounding {
            alignas(16) glm::vec3 min;
            alignas(16) glm::vec3 max;
        };

        glm::mat4 model;
        glm::mat4 normal;
        std::uint32_t materialIdx;
        Bounding bounding;
        bool isCulled;
    };

    struct Info {
        ResourceID resourceIdPrefix;
    };

    BatchGenerator(const Info &info);

    bool build(const std::unordered_set<ObjectID> &objects);
    bool destroy();

    void draw(const RenderContext &context);

    const Resource<DynamicBuffer> getIndirectBuffer() const;
    const Resource<DynamicBuffer> getSceneBuffer() const;

    const RenderNode &getRenderNode() const;
    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderNode m_RenderNode;

    std::vector<SceneDataBuffer> m_SceneBuffer;
    std::vector<vk::DrawIndexedIndirectCommand> m_IndirectBuffer;
    std::vector<std::tuple<Resource<Mesh>, std::size_t>> m_IndirectMeshes;

    Resource<DynamicBuffer> m_Scene;
    Resource<DynamicBuffer> m_Indirect;
};

} // namespace Physbuzz
