#pragma once

#include "../containers/directedgraph.hpp"
#include "../resources/defines.hpp"
#include "descriptors/dynamic.hpp"
#include "descriptors/texture.hpp"

namespace Physbuzz {

using RenderNodeID = std::string;

struct RenderNode {
    template <typename T, typename... Args>
        requires ResourceBuildableType<T, Args...>
    struct DescriptorUsage {
        std::unordered_set<ResourceID> input = {};
        std::unordered_map<ResourceID, std::tuple<typename T::Info, Args...>> output = {};
    };

    struct Description {
        DescriptorUsage<Texture, glm::uvec3> textures = {};
        DescriptorUsage<DynamicBuffer, std::uint64_t> buffers = {};
    } description = {};

    std::function<void(Scene *, const RenderContext &)> execute;
};

class RenderGraph {
  public:
    struct Resources {
        std::unordered_set<ResourceID> textures;
        std::unordered_set<ResourceID> buffers;
    };

    const RenderNode &add(const RenderNodeID &id, const RenderNode &node);
    const RenderNode &get(const RenderNodeID &id) const;

    bool compile(const RenderNodeID &outputId);
    void execute(Scene *scene, const RenderContext &context) const;

    const Resources &getResources() const;

  private:
    std::unordered_map<RenderNodeID, RenderNode> m_Nodes;
    DirectedGraph<RenderNodeID> m_Graph;

    std::vector<RenderNode> m_ExecutableNodes;
    Resources m_Resources;
};

} // namespace Physbuzz
