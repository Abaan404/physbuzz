#include "rendergraph.hpp"

#include "../resources/registry.hpp"
#include <tuple>

namespace Physbuzz {

const RenderNode &RenderGraph::add(const RenderNodeID &id, const RenderNode &node) {
    m_Nodes[id] = node;
    return m_Nodes.at(id);
}

const RenderNode &RenderGraph::get(const RenderNodeID &id) const {
    PBZ_ASSERT(m_Nodes.contains(id), std::format("[RenderGraph] RenderNodeID {} does not exist in this graph.", id));
    return m_Nodes.at(id);
}

bool RenderGraph::compile(const RenderNodeID &outputId) {
    PBZ_ASSERT(m_Nodes.contains(outputId), "[RenderGraph] Graph's outputId is not present.");

    m_Graph.clear();

    m_ExecutableNodes.clear();
    m_Resources.textures.clear();
    m_Resources.buffers.clear();

    std::unordered_map<ResourceID, std::unordered_set<RenderNodeID>> textureReaders;
    std::unordered_map<ResourceID, std::unordered_set<RenderNodeID>> bufferReaders;
    m_Graph.insertNode(outputId);

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
    m_Graph.cull(outputId);

    m_ExecutableNodes.reserve(m_Graph.size());

    // generate order and barriers
    for (const auto &id : m_Graph.sort()) {
        m_ExecutableNodes.emplace_back(m_Nodes.at(id));
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
            m_Resources.textures.emplace(resource);
        }
    }

    return success;
}

void RenderGraph::execute(Scene *scene, const RenderContext &context) const {
    for (auto &node : m_ExecutableNodes) {
        node.execute(scene, context);
    }
}

const RenderGraph::Resources &RenderGraph::getResources() const {
    return m_Resources;
}

} // namespace Physbuzz
