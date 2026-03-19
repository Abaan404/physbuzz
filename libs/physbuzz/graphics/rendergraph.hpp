#pragma once

#include "../containers/directedgraph.hpp"
#include "../resources/resource.hpp"
#include "descriptors/attachment.hpp"
#include "descriptors/dynamic.hpp"

namespace Physbuzz {

class Scene;

using RenderNodeID = std::string;

struct RenderNode {
    enum class Stage {
        Indirect,
        Vertex,
        Fragment,
        Graphics,
        Compute,
        Transfer,
    };

    struct BufferDesc {
        Stage stage;
    };

    struct AttachmentDesc {
        Stage stage;
    };

    template <ResourceType T, typename D>
    struct ResourceUsage {
        std::unordered_map<Resource<T>, D> input = {};
        std::unordered_map<Resource<T>, D> output = {};
    };

    struct {
        ResourceUsage<DynamicBuffer, BufferDesc> buffers;
        ResourceUsage<Attachment, AttachmentDesc> attachments;
    } description = {};

    std::function<void(Scene *, const RenderContext &)> prepare;
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
    struct Barriers {
        std::vector<std::tuple<vk::BufferMemoryBarrier2, Resource<DynamicBuffer>>> m_BufferBarriers;
        std::vector<std::tuple<vk::ImageMemoryBarrier2, Resource<Attachment>>> m_AttachmentBarriers;

        void apply(const RenderContext &context) const;
    };

    Info m_Info;

    std::unordered_map<RenderNodeID, RenderNode> m_Nodes;
    DirectedGraph<RenderNodeID> m_Graph;

    std::vector<RenderNode> m_ExecutableNodes;
    std::vector<RenderNodeID> m_ExecutableNodeIds;
    std::vector<Barriers> m_ExecutableNodeBarriers;

    Resources m_Resources;
};

} // namespace Physbuzz
