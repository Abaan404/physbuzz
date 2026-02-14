#include "rendergraph.hpp"

#include "../resources/registry.hpp"
#include <tuple>

namespace Physbuzz {

RenderGraph::RenderGraph(const Info &info)
    : m_Info(info) {}

const RenderNode &RenderGraph::add(const RenderNodeID &id, const RenderNode &node) {
    m_Nodes[id] = node;
    return m_Nodes.at(id);
}

const RenderNode &RenderGraph::get(const RenderNodeID &id) const {
    PBZ_ASSERT(m_Nodes.contains(id), std::format("[RenderGraph] RenderNodeID {} does not exist in this graph.", id));
    return m_Nodes.at(id);
}

void RenderGraph::merge(const RenderGraph &graph) {
    m_Graph.merge(graph.m_Graph);
    m_Nodes.insert(graph.m_Nodes.begin(), graph.m_Nodes.end());

    if (!m_Info.output.empty()) {
        m_Graph.insertEdge(m_Info.output, graph.getInfo().output);
    }

    m_Info.output = graph.getInfo().output;

    // merging requires a recompile
    m_ExecutableNodes.clear();
    m_ExecutableNodeIds.clear();
    m_Resources.buffers.clear();
    m_Resources.textures.clear();
}

bool RenderGraph::compile() {
    PBZ_ASSERT(m_Nodes.contains(m_Info.output), std::format("[RenderGraph] Graph's outputId '{}' is not present.", m_Info.output));

    m_ExecutableNodes.clear();
    m_ExecutableNodeIds.clear();
    m_Resources.textures.clear();
    m_Resources.buffers.clear();

    std::unordered_map<ResourceID, std::unordered_set<RenderNodeID>> textureReaders;
    std::unordered_map<ResourceID, std::unordered_set<RenderNodeID>> bufferReaders;

    for (auto &[inputId, node] : m_Nodes) {
        for (auto &resource : node.description.buffers.input) {
            bufferReaders.try_emplace(resource);
            bufferReaders.at(resource).emplace(inputId);
        }

        for (auto &resource : node.description.textures.input) {
            textureReaders.try_emplace(resource);
            textureReaders.at(resource).emplace(inputId);
        }
    }

    // build the graph
    for (auto &[outputId, node] : m_Nodes) {
        for (auto &[resource, _] : node.description.buffers.output) {
            // unused resource
            if (!bufferReaders.contains(resource)) {
                continue;
            }

            for (const auto &inputId : bufferReaders.at(resource)) {
                m_Graph.insertEdge(outputId, inputId);
            }
        }

        for (auto &[resource, _] : node.description.textures.output) {
            // unused resource
            if (!textureReaders.contains(resource)) {
                continue;
            }

            for (const auto &inputId : textureReaders.at(resource)) {
                m_Graph.insertEdge(outputId, inputId);
            }
        }
    }

    // eliminate nodes that do not contribute to the executable graph
    m_Graph.cull(m_Info.output);

    m_ExecutableNodes.reserve(m_Graph.size());
    m_ExecutableNodeIds.reserve(m_Graph.size());

    // generate order and barriers
    for (const auto &id : m_Graph.sort()) {
        m_ExecutableNodes.emplace_back(m_Nodes.at(id));
        m_ExecutableNodeIds.emplace_back(id);
    }

    bool success = true;

    // copy and build resources
    for (auto &node : m_ExecutableNodes) {
        for (auto &[resource, tuple] : node.description.textures.output) {
            if (!ResourceRegistry<Texture>::contains(resource)) {
                success &= ResourceRegistry<Texture>::insert(resource, std::get<0>(tuple), std::get<1>(tuple));
            }
            m_Resources.textures.emplace(resource);
        }

        for (auto &[resource, tuple] : node.description.buffers.output) {
            if (!ResourceRegistry<DynamicBuffer>::contains(resource)) {
                success &= ResourceRegistry<DynamicBuffer>::insert(resource, std::get<0>(tuple), std::get<1>(tuple));
            }
            m_Resources.buffers.emplace(resource);
        }
    }

    return success;
}

void RenderGraph::execute(Scene *scene, const RenderContext &context) const {
    for (auto &node : m_ExecutableNodes) {
        node.execute(scene, context);
    }
}

const std::vector<RenderNodeID> &RenderGraph::getExecutableNodes() const {
    return m_ExecutableNodeIds;
}

const RenderGraph::Resources &RenderGraph::getResources() const {
    return m_Resources;
}

const RenderGraph::Info &RenderGraph::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
