#pragma once

#include "../containers/directedgraph.hpp"
#include "../resources/resource.hpp"
#include "descriptors/attachment.hpp"
#include "descriptors/dynamic.hpp"
#include "descriptors/texture.hpp"

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

    template <ResourceType T>
    struct Description;

    template <>
    struct Description<DynamicBuffer> {
        using Resource = Resource<DynamicBuffer>;
        using Desc = struct {
            Stage stage;
        };
    };

    template <>
    struct Description<Attachment> {
        using Resource = Resource<Attachment>;
        using Desc = struct {
            Stage stage;
            std::vector<Image::SubresourceRange> subresourceRanges;
        };
    };

    template <>
    struct Description<Texture> {
        using Resource = Resource<Texture>;
        using Desc = struct {
            Stage stage;
            std::vector<Image::SubresourceRange> subresourceRanges;
        };
    };

    template <ResourceType T>
    struct DescriptionMap {
        using InfoType = Description<T>;

        std::unordered_map<typename InfoType::Resource, typename InfoType::Desc> input = {};
        std::unordered_map<typename InfoType::Resource, typename InfoType::Desc> output = {};
    };

    struct Descriptions {
        DescriptionMap<DynamicBuffer> buffers;
        DescriptionMap<Attachment> attachments;
        DescriptionMap<Texture> textures;
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
        std::unordered_set<Resource<Texture>> textures;
    };

    const RenderNode &add(const RenderNodeID &id, const RenderNode &node);
    const RenderNode &get(const RenderNodeID &id) const;
    bool contains(const RenderNodeID &id) const;

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
        std::vector<std::tuple<vk::ImageMemoryBarrier2, Resource<Texture>>> m_TextureBarriers;

        void apply(const RenderContext &context) const;
    };

    Info m_Info;

    std::unordered_map<RenderNodeID, RenderNode> m_Nodes;
    std::vector<RenderNodeID> m_OrderedNodes;
    std::vector<RenderNodeID> m_OutputNodes;

    DirectedGraph<RenderNodeID> m_Graph;

    std::vector<RenderNode> m_ExecutableNodes;
    std::vector<RenderNodeID> m_ExecutableNodeIds;
    std::vector<Barriers> m_ExecutableNodeBarriers;

    Resources m_Resources;

    template <ResourceType T>
    void setupGraphForResource();

    template <ResourceType T>
    bool setupBarriersForResource(const std::function<bool(
                                      Barriers &barriers,
                                      const RenderNode::DescriptionMap<T> &map,
                                      const typename RenderNode::Description<T>::Resource &resource,
                                      const typename RenderNode::Description<T>::Desc &desc)> &callback);
};

} // namespace Physbuzz
