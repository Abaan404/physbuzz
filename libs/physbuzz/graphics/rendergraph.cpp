#include "rendergraph.hpp"

#include "../ecs/scene.hpp"
#include "../resources/registry.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

RenderGraph::RenderGraph(const Info &info)
    : m_Info(info) {
    // constructing a new graph will have only one output, if the output is empty
    // the graph is considered invalid
    if (!info.output.empty()) {
        m_OutputNodes = {info.output};
    }
}

const RenderNode &RenderGraph::add(const RenderNodeID &id, const RenderNode &node) {
    m_Nodes[id] = node;
    m_OrderedNodes.emplace_back(id);
    return m_Nodes.at(id);
}

const RenderNode &RenderGraph::get(const RenderNodeID &id) const {
    PBZ_ASSERT(m_Nodes.contains(id), std::format("[RenderGraph] RenderNodeID {} does not exist in this graph.", id));
    return m_Nodes.at(id);
}

bool RenderGraph::contains(const RenderNodeID &id) const {
    return m_Nodes.contains(id);
}

void RenderGraph::merge(const RenderGraph &graph) {
    for (const auto &nodeId : graph.m_OrderedNodes) {
        if (!m_Nodes.contains(nodeId)) {
            m_Nodes.emplace(nodeId, graph.m_Nodes.at(nodeId));
            m_OrderedNodes.emplace_back(nodeId);
        }
    }

    if (!graph.m_Info.output.empty()) {
        m_Info.output = graph.m_Info.output;
        m_OutputNodes.emplace_back(graph.m_Info.output);
    }

    // merging requires a recompile
    m_Graph.clear();
    m_ExecutableNodes.clear();
    m_ExecutableNodeIds.clear();
    m_ExecutableNodeBarriers.clear();
    m_Resources.buffers.clear();
    m_Resources.attachments.clear();
}

bool RenderGraph::compile() {
    PBZ_ASSERT(m_Nodes.contains(m_Info.output), std::format("[RenderGraph] Graph's outputId '{}' is not present.", m_Info.output));

    m_Graph.clear();
    m_ExecutableNodes.clear();
    m_ExecutableNodeIds.clear();
    m_ExecutableNodeBarriers.clear();
    m_Resources.buffers.clear();
    m_Resources.attachments.clear();

    setupGraphForResource<DynamicBuffer>();
    setupGraphForResource<Attachment>();
    setupGraphForResource<Texture>();

    // if there are any merged graphs, respect the output order and add edges
    for (std::size_t i = 0; i < m_OutputNodes.size() - 1; i++) {
        m_Graph.insertEdge(m_OutputNodes[i], m_OutputNodes[i + 1]);
    }

    // eliminate nodes that do not contribute to the executable graph
    m_Graph.cull(m_Info.output);

    bool success = true;

    m_ExecutableNodeIds = m_Graph.sort();

    m_ExecutableNodes.reserve(m_ExecutableNodeIds.size());
    m_ExecutableNodeBarriers.reserve(m_ExecutableNodeIds.size());

    // build node resources
    for (const auto &id : m_ExecutableNodeIds) {
        RenderNode &node = m_Nodes.at(id);

        for (auto &[resource, data] : node.description.buffers.output) {
            if (!ResourceRegistry<DynamicBuffer>::contains(resource)) {
                Logger::ERROR("[RenderGraph] DynamicBuffer '{}' does not exist required by node '{}'", resource, id);
                return false;
            }

            m_Resources.buffers.emplace(resource);
        }

        for (auto &[resource, data] : node.description.attachments.output) {
            if (!ResourceRegistry<Attachment>::contains(resource)) {
                Logger::ERROR("[RenderGraph] Attachment '{}' does not exist required by node '{}'", resource, id);
                return false;
            }

            m_Resources.attachments.emplace(resource);
        }

        for (auto &[resource, data] : node.description.textures.output) {
            if (!ResourceRegistry<Texture>::contains(resource)) {
                Logger::ERROR("[RenderGraph] Texture '{}' does not exist required by node '{}'", resource, id);
                return false;
            }

            m_Resources.textures.emplace(resource);
        }
    }

    // build executable vector
    for (const auto &id : m_ExecutableNodeIds) {
        RenderNode &node = m_ExecutableNodes.emplace_back(m_Nodes.at(id));

        // give invalid functions a noop
        if (!node.prepare) {
            node.prepare = [](Scene *scene, const RenderContext &context) {
            };
        }

        if (!node.execute) {
            node.execute = [](Scene *scene, const RenderContext &context) {
            };
        }
    }

    std::unordered_map<ResourceID, std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2>> prevBufferBarrier;

    success &= setupBarriersForResource<DynamicBuffer>([&prevBufferBarrier](auto &barriers, const auto &map, const auto &buffer, const auto &desc) {
        if (!prevBufferBarrier.contains(buffer)) {
            prevBufferBarrier[buffer] = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};
        }

        std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2> nextBufferBarrier = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};

        // select pipeline stage flags
        vk::PipelineStageFlags2 nextStage = {};
        switch (desc.stage) {
        case RenderNode::Stage::Indirect:
            nextStage = vk::PipelineStageFlagBits2::eDrawIndirect;
            break;

        case RenderNode::Stage::Vertex:
            nextStage = vk::PipelineStageFlagBits2::eVertexShader;
            break;

        case RenderNode::Stage::Fragment:
            nextStage = vk::PipelineStageFlagBits2::eFragmentShader;
            break;

        case RenderNode::Stage::Compute:
            nextStage = vk::PipelineStageFlagBits2::eComputeShader;
            break;

        case RenderNode::Stage::Transfer:
            nextStage = vk::PipelineStageFlagBits2::eTransfer;
            break;

        case RenderNode::Stage::Graphics:
            nextStage = vk::PipelineStageFlagBits2::eAllGraphics;
            break;
        }

        // select access flags
        vk::AccessFlags2 nextAccess = {};

        // read buffers
        if (map.input.contains(buffer)) {
            switch (desc.stage) {
            case RenderNode::Stage::Indirect:
            case RenderNode::Stage::Vertex:
            case RenderNode::Stage::Fragment:
            case RenderNode::Stage::Graphics:
                switch (buffer->getInfo().type) {
                case DynamicBuffer::Type::Indirect:
                    nextAccess |= vk::AccessFlagBits2::eIndirectCommandRead;
                    break;

                case DynamicBuffer::Type::Constant:
                case DynamicBuffer::Type::ConstantDynamic:
                    nextAccess |= vk::AccessFlagBits2::eUniformRead;
                    break;

                case DynamicBuffer::Type::Structured:
                case DynamicBuffer::Type::StructuredDynamic:
                    nextAccess |= vk::AccessFlagBits2::eShaderStorageRead;
                    break;
                }
                break;
            case RenderNode::Stage::Compute:
                switch (buffer->getInfo().type) {
                case DynamicBuffer::Type::Constant:
                case DynamicBuffer::Type::ConstantDynamic:
                    nextAccess |= vk::AccessFlagBits2::eUniformRead;
                    break;

                case DynamicBuffer::Type::Indirect:
                case DynamicBuffer::Type::Structured:
                case DynamicBuffer::Type::StructuredDynamic:
                    nextAccess |= vk::AccessFlagBits2::eShaderStorageRead;
                    break;
                }
                break;

            case RenderNode::Stage::Transfer:
                nextAccess |= vk::AccessFlagBits2::eTransferRead;
                break;
            }
        }

        // write buffers
        if (map.output.contains(buffer)) {
            switch (desc.stage) {
            case RenderNode::Stage::Indirect:
            case RenderNode::Stage::Vertex:
            case RenderNode::Stage::Fragment:
            case RenderNode::Stage::Graphics:
            case RenderNode::Stage::Compute:
                switch (buffer->getInfo().type) {
                case DynamicBuffer::Type::Constant:
                case DynamicBuffer::Type::ConstantDynamic:
                    Logger::ERROR("[RenderGraph] Cannot write to a uniform buffer");
                    return false;

                case DynamicBuffer::Type::Indirect:
                case DynamicBuffer::Type::Structured:
                case DynamicBuffer::Type::StructuredDynamic:
                    nextAccess |= vk::AccessFlagBits2::eShaderStorageWrite;
                    break;
                }
                break;

            case RenderNode::Stage::Transfer:
                nextAccess |= vk::AccessFlagBits2::eTransferWrite;
                break;
            }
        }

        // assign as next
        nextBufferBarrier = {nextStage, nextAccess};

        // skip read-on-read hazards
        auto [_, prevAccess] = prevBufferBarrier.at(buffer);

        bool prevIsRead = !static_cast<bool>(prevAccess & vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eTransferWrite);
        bool nextIsRead = !static_cast<bool>(nextAccess & vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eTransferWrite);

        if (prevIsRead && nextIsRead) {
            return true;
        }

        // store barrier
        barriers.m_BufferBarriers.emplace_back(std::make_tuple(
            vk::BufferMemoryBarrier2{
                .srcStageMask = std::get<0>(prevBufferBarrier.at(buffer)),
                .srcAccessMask = std::get<1>(prevBufferBarrier.at(buffer)),
                .dstStageMask = std::get<0>(nextBufferBarrier),
                .dstAccessMask = std::get<1>(nextBufferBarrier),
                .offset = 0,
                .size = vk::WholeSize,
            },
            buffer));

        prevBufferBarrier[buffer] = nextBufferBarrier;

        return true;
    });

    std::unordered_map<ResourceID, std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2>> prevAttachmentBarrier;
    std::unordered_map<ResourceID, vk::ImageLayout> prevAttachmentLayout;

    success &= setupBarriersForResource<Attachment>([&prevAttachmentBarrier, &prevAttachmentLayout](auto &barriers, const auto &map, const auto &attachment, const auto &desc) {
        if (!prevAttachmentBarrier.contains(attachment)) {
            prevAttachmentBarrier[attachment] = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};
        }

        if (!prevAttachmentLayout.contains(attachment)) {
            prevAttachmentLayout[attachment] = vk::ImageLayout::eUndefined;
        }

        std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2> nextAttachmentBarrier = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};

        // select acces and layout
        vk::AccessFlags2 nextAccess = {};
        vk::ImageLayout nextLayout = {};
        vk::PipelineStageFlags2 nextStage = {};

        // read attachment
        if (map.input.contains(attachment)) {
            switch (desc.stage) {
            case RenderNode::Stage::Indirect:
                Logger::ERROR("[RenderGraph] Cannot use attachment in Indirect stage.");
                return false;

            case RenderNode::Stage::Vertex:
            case RenderNode::Stage::Compute:
                Logger::ERROR("[RenderGraph] Cannot use attachment in Vertex or Compute stage.");
                return false;

            case RenderNode::Stage::Fragment:
            case RenderNode::Stage::Graphics:
                switch (attachment->getInfo().usage) {
                case Attachment::Usage::Color:
                    nextAccess |= vk::AccessFlagBits2::eInputAttachmentRead;
                    break;

                case Attachment::Usage::Depth:
                case Attachment::Usage::Stencil:
                case Attachment::Usage::DepthStencil:
                    nextAccess |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;
                    break;
                }

                switch (attachment->getInfo().usage) {
                case Attachment::Usage::Color:
                    nextLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                    break;

                case Attachment::Usage::Depth:
                    nextLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                    break;

                case Attachment::Usage::Stencil:
                    nextLayout = vk::ImageLayout::eStencilReadOnlyOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                    break;

                case Attachment::Usage::DepthStencil:
                    nextLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                    break;
                }

                if (attachment->getInfo().sampler.getInfo().type != Sampler::Type::None) {
                    nextLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                    nextAccess = vk::AccessFlagBits2::eShaderRead;
                }
                break;

            case RenderNode::Stage::Transfer:
                nextAccess |= vk::AccessFlagBits2::eTransferRead;
                nextLayout = vk::ImageLayout::eTransferSrcOptimal;
                nextStage |= vk::PipelineStageFlagBits2::eTransfer;
                break;
            }
        }

        // write attachment
        if (map.output.contains(attachment)) {
            switch (desc.stage) {
            case RenderNode::Stage::Indirect:
                Logger::ERROR("[RenderGraph] Cannot use attachment in Indirect stage.");
                return false;

            case RenderNode::Stage::Vertex:
            case RenderNode::Stage::Compute:
                Logger::ERROR("[RenderGraph] Cannot use attachment in Vertex or Compute stage.");
                return false;

            case RenderNode::Stage::Graphics:
            case RenderNode::Stage::Fragment:
                switch (attachment->getInfo().usage) {
                case Attachment::Usage::Color:
                    nextAccess |= vk::AccessFlagBits2::eColorAttachmentWrite;
                    break;

                case Attachment::Usage::Depth:
                case Attachment::Usage::Stencil:
                case Attachment::Usage::DepthStencil:
                    nextAccess |= vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
                    break;
                }

                switch (attachment->getInfo().usage) {
                case Attachment::Usage::Color:
                    nextLayout = vk::ImageLayout::eAttachmentOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                    break;

                case Attachment::Usage::Depth:
                    nextLayout = vk::ImageLayout::eDepthAttachmentOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
                    break;

                case Attachment::Usage::Stencil:
                    nextLayout = vk::ImageLayout::eStencilAttachmentOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
                    break;

                case Attachment::Usage::DepthStencil:
                    nextLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                    nextStage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
                    break;
                }

                break;

            case RenderNode::Stage::Transfer:
                nextAccess |= vk::AccessFlagBits2::eTransferWrite;
                nextLayout = vk::ImageLayout::eTransferDstOptimal;
                nextStage |= vk::PipelineStageFlagBits2::eTransfer;
                break;
            }
        }

        nextAttachmentBarrier = {nextStage, nextAccess};

        // skip read-on-read hazards
        if (prevAttachmentBarrier.at(attachment) == nextAttachmentBarrier) {
            return true;
        }

        for (const auto &subresourceRange : desc.subresourceRanges) {
            // store barrier
            barriers.m_AttachmentBarriers.emplace_back(std::make_tuple(
                vk::ImageMemoryBarrier2{
                    .srcStageMask = std::get<0>(prevAttachmentBarrier.at(attachment)),
                    .srcAccessMask = std::get<1>(prevAttachmentBarrier.at(attachment)),
                    .dstStageMask = std::get<0>(nextAttachmentBarrier),
                    .dstAccessMask = std::get<1>(nextAttachmentBarrier),
                    .oldLayout = prevAttachmentLayout[attachment],
                    .newLayout = nextLayout,
                    .subresourceRange = subresourceRange,
                },
                attachment));
        }

        prevAttachmentBarrier[attachment] = nextAttachmentBarrier;
        prevAttachmentLayout[attachment] = nextLayout;
        return true;
    });

    std::unordered_map<ResourceID, std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2>> prevTextureBarrier;
    std::unordered_map<ResourceID, vk::ImageLayout> prevTextureLayout;

    success &= setupBarriersForResource<Texture>([&prevTextureBarrier, &prevTextureLayout](auto &barriers, const auto &map, const auto &texture, const auto &desc) {
        if (!prevTextureBarrier.contains(texture)) {
            prevTextureBarrier[texture] = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};
        }

        if (!prevTextureLayout.contains(texture)) {
            prevTextureLayout[texture] = vk::ImageLayout::eUndefined;
        }

        std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2> nextTextureBarrier = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};

        // select acces and layout
        vk::AccessFlags2 nextAccess = {};
        vk::ImageLayout nextLayout = {};
        vk::PipelineStageFlags2 nextStage = {};

        // read texture
        if (map.input.contains(texture)) {
            switch (desc.stage) {
            case RenderNode::Stage::Indirect:
                Logger::ERROR("[RenderGraph] Cannot use texture in Indirect stage.");
                return false;

            case RenderNode::Stage::Vertex:
                Logger::ERROR("[RenderGraph] Cannot use texture in Vertex stage.");
                return false;

            case RenderNode::Stage::Compute:
                nextAccess |= vk::AccessFlagBits2::eShaderRead;
                nextLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                nextStage = vk::PipelineStageFlagBits2::eComputeShader;
                break;

            case RenderNode::Stage::Fragment:
            case RenderNode::Stage::Graphics:
                nextAccess |= vk::AccessFlagBits2::eShaderRead;
                nextLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                nextStage = vk::PipelineStageFlagBits2::eFragmentShader;
                break;

            case RenderNode::Stage::Transfer:
                nextAccess |= vk::AccessFlagBits2::eTransferRead;
                nextLayout = vk::ImageLayout::eTransferSrcOptimal;
                nextStage = vk::PipelineStageFlagBits2::eTransfer;
                break;
            }
        }

        // write texture
        if (map.output.contains(texture)) {
            switch (desc.stage) {
            case RenderNode::Stage::Indirect:
                Logger::ERROR("[RenderGraph] Cannot use texture in Indirect stage.");
                return false;

            case RenderNode::Stage::Vertex:
                Logger::ERROR("[RenderGraph] Cannot use texture in Vertex or Compute stage.");
                return false;

            case RenderNode::Stage::Compute:
                nextAccess |= vk::AccessFlagBits2::eShaderWrite;
                nextLayout = vk::ImageLayout::eGeneral;
                nextStage |= vk::PipelineStageFlagBits2::eComputeShader;
                break;

            case RenderNode::Stage::Fragment:
            case RenderNode::Stage::Graphics:
                nextAccess |= vk::AccessFlagBits2::eShaderWrite;
                nextLayout = vk::ImageLayout::eGeneral;
                nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                break;

            case RenderNode::Stage::Transfer:
                nextLayout = vk::ImageLayout::eTransferDstOptimal;
                nextAccess |= vk::AccessFlagBits2::eTransferWrite;
                nextStage |= vk::PipelineStageFlagBits2::eTransfer;
                break;
            }
        }

        nextTextureBarrier = {nextStage, nextAccess};

        // skip read-on-read hazards
        if (prevTextureBarrier.at(texture) == nextTextureBarrier) {
            return true;
        }

        for (const auto &subresourceRange : desc.subresourceRanges) {
            // store barrier
            barriers.m_TextureBarriers.emplace_back(std::make_tuple(
                vk::ImageMemoryBarrier2{
                    .srcStageMask = std::get<0>(prevTextureBarrier.at(texture)),
                    .srcAccessMask = std::get<1>(prevTextureBarrier.at(texture)),
                    .dstStageMask = std::get<0>(nextTextureBarrier),
                    .dstAccessMask = std::get<1>(nextTextureBarrier),
                    .oldLayout = prevTextureLayout[texture],
                    .newLayout = nextLayout,
                    .subresourceRange = subresourceRange,
                },
                texture));
        }

        prevTextureBarrier[texture] = nextTextureBarrier;
        prevTextureLayout[texture] = nextLayout;
        return true;
    });

    return success;
}

void RenderGraph::execute(Scene *scene, const RenderContext &context) const {
    // prepare any nodes for rebuilding, validating, etc
    {
        ZoneScopedN("RenderGraph/Prepare");
        for (std::size_t i = 0; i < m_ExecutableNodeIds.size(); i++) {
            m_ExecutableNodes[i].prepare(scene, context);
        }
    }

    // insert barriers and execute
    {
        ZoneScopedN("RenderGraph/Execute");
        for (std::size_t i = 0; i < m_ExecutableNodeIds.size(); i++) {
            m_ExecutableNodeBarriers[i].apply(context);
            m_ExecutableNodes[i].execute(scene, context);
        }
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

template <ResourceType T>
void RenderGraph::setupGraphForResource() {
    enum class UsageFlagBits : uint32_t {
        Read = 1 << 0,
        Write = 1 << 1,
    };

    std::unordered_map<ResourceID, std::unordered_map<RenderNodeID, vk::Flags<UsageFlagBits>>> usageUnordered;
    std::unordered_map<ResourceID, std::vector<std::tuple<RenderNodeID, vk::Flags<UsageFlagBits>>>> usageOrdered;

    // invert the stored nodes to lookup by resource usage
    for (const auto &[nodeId, node] : m_Nodes) {
        // FIXME a lil silly but this can be improved with cpp26 metaprogramming I'd imagine
        const RenderNode::DescriptionMap<T> &map = [&node]() {
            if constexpr (std::is_same_v<DynamicBuffer, T>) {
                return node.description.buffers;
            } else if constexpr (std::is_same_v<Attachment, T>) {
                return node.description.attachments;
            } else if constexpr (std::is_same_v<Texture, T>) {
                return node.description.textures;
            } else {
                static_assert(false, "unsupported resource type for RenderNode::Description");
            }
        }();

        for (auto &[resource, desc] : map.input) {
            usageUnordered[resource][nodeId] |= UsageFlagBits::Read;
        }

        for (auto &[resource, desc] : map.output) {
            usageUnordered[resource][nodeId] |= UsageFlagBits::Write;
        }
    }

    // reserve allocations
    for (auto &[resource, nodeUsage] : usageUnordered) {
        usageOrdered[resource].reserve(nodeUsage.size());
    }

    // since its an unordered map, turn it into a vector that can preserve
    // the initial order that the graph will use to build the map
    for (const auto &nodeId : m_OrderedNodes) {
        for (auto &[resource, nodeUsage] : usageUnordered) {
            if (nodeUsage.contains(nodeId)) {
                usageOrdered[resource].emplace_back(nodeId, nodeUsage.at(nodeId));
            }
        }
    }

    // build the graph
    for (const auto &[resource, nodeUsage] : usageOrdered) {
        RenderNodeID lastWriterId;

        for (const auto &[nodeId, usage] : nodeUsage) {
            // track this node if future reads's current node doesnt write this resource
            if (usage & UsageFlagBits::Read) {
                // cannot build a relationship if the first node needs a read
                if (lastWriterId.empty()) {
                    Logger::WARNING("[RenderGraph] Node '{}' reads a texture '{}' that has not been written yet.", nodeId, resource);
                    continue;
                }

                m_Graph.insertEdge(lastWriterId, nodeId);
            }

            if (usage & UsageFlagBits::Write) {
                lastWriterId = nodeId;

                if (m_Info.output == nodeId) {
                    m_Graph.insertNode(nodeId);
                }
            }
        }
    }
}

template <ResourceType T>
bool RenderGraph::setupBarriersForResource(const std::function<bool(
                                               Barriers &barriers,
                                               const RenderNode::DescriptionMap<T> &map,
                                               const typename RenderNode::Description<T>::Resource &resource,
                                               const typename RenderNode::Description<T>::Desc &desc)> &callback) {
    bool success = true;

    for (const auto &id : m_ExecutableNodeIds) {
        RenderNode &node = m_Nodes.at(id);
        Barriers &barriers = m_ExecutableNodeBarriers.emplace_back<Barriers>({});

        // collect all resources
        std::vector<std::tuple<
            typename RenderNode::Description<T>::Resource,
            typename RenderNode::Description<T>::Desc>>
            resources;

        // FIXME a lil silly but this can be improved with cpp26 metaprogramming I'd imagine
        const RenderNode::DescriptionMap<T> &map = [&node]() {
            if constexpr (std::is_same_v<DynamicBuffer, T>) {
                return node.description.buffers;
            } else if constexpr (std::is_same_v<Attachment, T>) {
                return node.description.attachments;
            } else if constexpr (std::is_same_v<Texture, T>) {
                return node.description.textures;
            } else {
                static_assert(false, "unsupported resource type for RenderNode::Description");
            }
        }();

        for (const auto &[resource, desc] : map.input) {
            resources.emplace_back(std::make_tuple(resource, desc));
        }

        for (const auto &[resource, desc] : map.output) {
            resources.emplace_back(std::make_tuple(resource, desc));
        }

        for (const auto &[resource, desc] : resources) {
            success &= callback(barriers, map, resource, desc);
        }
    }

    return success;
}

void RenderGraph::Barriers::apply(const RenderContext &context) const {
    std::vector<vk::BufferMemoryBarrier2> bufferBarriers;
    std::vector<vk::ImageMemoryBarrier2> attachmentBarriers;

    bufferBarriers.reserve(m_BufferBarriers.size());
    attachmentBarriers.reserve(m_AttachmentBarriers.size());

    for (const auto &[barrier, buffer] : m_BufferBarriers) {
        vk::BufferMemoryBarrier2 &elem = bufferBarriers.emplace_back(barrier);

        elem.buffer = buffer->getRingData()[context.frameInFlight].buffer.getData().buffer;
    }

    for (const auto &[barrier, attachment] : m_AttachmentBarriers) {
        vk::ImageMemoryBarrier2 &elem = attachmentBarriers.emplace_back(barrier);

        elem.image = attachment->getRingData()[context.frameInFlight].image.getData().image;
    }

    for (const auto &[barrier, texture] : m_TextureBarriers) {
        vk::ImageMemoryBarrier2 &elem = attachmentBarriers.emplace_back(barrier);

        elem.image = texture->getData().image.getData().image;
    }

    context.command.pipelineBarrier2({
        .dependencyFlags = {},
        .bufferMemoryBarrierCount = static_cast<std::uint32_t>(bufferBarriers.size()),
        .pBufferMemoryBarriers = bufferBarriers.data(),
        .imageMemoryBarrierCount = static_cast<std::uint32_t>(attachmentBarriers.size()),
        .pImageMemoryBarriers = attachmentBarriers.data(),
    });
}

} // namespace Physbuzz
