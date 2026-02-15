#pragma once

#include "../containers/directedgraph.hpp"
#include "../resources/resource.hpp"
#include "descriptors/attachment.hpp"
#include "descriptors/dynamic.hpp"

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
        DescriptorUsage<Attachment, glm::uvec2> attachments = {};
        DescriptorUsage<DynamicBuffer, std::uint64_t> buffers = {};
    } description = {};

    std::function<void(Scene *, const RenderContext &)> execute;
};

class RenderGraph {
  public:
    struct Info {
        RenderNodeID output;
    };

    RenderGraph(const Info &info);

    struct Resources {
        std::unordered_set<Resource<Attachment>> attachments;
        std::unordered_set<Resource<DynamicBuffer>> buffers;
    };

    const RenderNode &add(const RenderNodeID &id, const RenderNode &node);
    const RenderNode &get(const RenderNodeID &id) const;

    void merge(const RenderGraph &graph);

    bool compile();
    void execute(Scene *scene, const RenderContext &context) const;

    const std::vector<RenderNodeID> &getExecutableNodes() const;
    const Resources &getResources() const;
    const Info &getInfo() const;

  private:
    Info m_Info;

    std::unordered_map<RenderNodeID, RenderNode> m_Nodes;
    DirectedGraph<RenderNodeID> m_Graph;

    std::vector<RenderNode> m_ExecutableNodes;
    std::vector<RenderNodeID> m_ExecutableNodeIds;
    Resources m_Resources;
};

} // namespace Physbuzz
