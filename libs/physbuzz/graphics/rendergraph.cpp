#include "rendergraph.hpp"

#include "../ecs/scene.hpp"
#include "../resources/registry.hpp"

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

    if (!m_Info.output.empty() && !graph.m_Info.output.empty()) {
        m_Graph.insertEdge(m_Info.output, graph.getInfo().output);
    }

    if (!graph.m_Info.output.empty()) {
        m_Info.output = graph.getInfo().output;
    }

    // merging requires a recompile
    m_ExecutableNodes.clear();
    m_ExecutableNodeIds.clear();
    m_Resources.buffers.clear();
    m_Resources.attachments.clear();
}

bool RenderGraph::compile() {
    PBZ_ASSERT(m_Nodes.contains(m_Info.output), std::format("[RenderGraph] Graph's outputId '{}' is not present.", m_Info.output));

    m_ExecutableNodes.clear();
    m_ExecutableNodeIds.clear();
    m_Resources.buffers.clear();
    m_Resources.attachments.clear();

    std::unordered_map<ResourceID, std::unordered_set<RenderNodeID>> attachmentReaders;
    std::unordered_map<ResourceID, std::unordered_set<RenderNodeID>> bufferReaders;

    for (auto &[inputId, node] : m_Nodes) {
        for (auto &[resource, desc] : node.description.buffers.input) {
            bufferReaders.try_emplace(resource);
            bufferReaders.at(resource).emplace(inputId);
        }

        for (auto &[resource, desc] : node.description.attachments.input) {
            attachmentReaders.try_emplace(resource);
            attachmentReaders.at(resource).emplace(inputId);
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

        for (auto &[resource, _] : node.description.attachments.output) {
            // unused resource
            if (!attachmentReaders.contains(resource)) {
                continue;
            }

            for (const auto &inputId : attachmentReaders.at(resource)) {
                m_Graph.insertEdge(outputId, inputId);
            }
        }
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

        for (auto &[resource, data] : node.description.attachments.output) {
            if (!ResourceRegistry<Attachment>::contains(resource)) {
                Logger::ERROR("[RenderGraph] DynamicBuffer '{}' does not exist required by node '{}'", resource, id);
                return false;
            }

            m_Resources.attachments.emplace(resource);
        }

        for (auto &[resource, data] : node.description.buffers.output) {
            if (!ResourceRegistry<DynamicBuffer>::contains(resource)) {
                Logger::ERROR("[RenderGraph] Attachment '{}' does not exist required by node '{}'", resource, id);
                return false;
            }

            m_Resources.buffers.emplace(resource);
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
    std::unordered_map<ResourceID, std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2>> prevAttachmentBarrier;
    std::unordered_map<ResourceID, vk::ImageLayout> prevAttachmentLayout;

    // build executable barriers
    for (const auto &id : m_ExecutableNodeIds) {
        RenderNode &node = m_Nodes.at(id);

        Barriers &barriers = m_ExecutableNodeBarriers.emplace_back<Barriers>({});

        // collect all resources
        std::vector<std::tuple<Resource<DynamicBuffer>, RenderNode::BufferDesc>> buffers;
        for (const auto &[buffer, desc] : node.description.buffers.input) {
            buffers.emplace_back(std::make_tuple(buffer, desc));
        }

        for (const auto &[buffer, desc] : node.description.buffers.output) {
            buffers.emplace_back(std::make_tuple(buffer, desc));
        }

        for (const auto &[buffer, desc] : buffers) {
            if (!prevBufferBarrier.contains(buffer)) {
                prevBufferBarrier[buffer] = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};
            }

            std::tuple<vk::PipelineStageFlags2, vk::AccessFlags2> nextBufferBarrier = {vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone};

            // select pipeline stage flags
            vk::PipelineStageFlags2 nextStage = {};
            switch (desc.stage) {
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
            if (node.description.buffers.input.contains(buffer)) {
                switch (desc.stage) {
                case RenderNode::Stage::Vertex:
                case RenderNode::Stage::Fragment:
                case RenderNode::Stage::Graphics:
                case RenderNode::Stage::Compute:
                    switch (buffer->getInfo().type) {
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

                case RenderNode::Stage::Transfer:
                    nextAccess |= vk::AccessFlagBits2::eTransferRead;
                    break;
                }
            }

            // write buffers
            if (node.description.buffers.output.contains(buffer)) {
                switch (desc.stage) {
                case RenderNode::Stage::Vertex:
                case RenderNode::Stage::Fragment:
                case RenderNode::Stage::Graphics:
                case RenderNode::Stage::Compute:
                    switch (buffer->getInfo().type) {
                    case DynamicBuffer::Type::Constant:
                    case DynamicBuffer::Type::ConstantDynamic:
                        Logger::ERROR("[RenderGraph] Cannot write to a uniform buffer");
                        return false;

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
            if (prevBufferBarrier.at(buffer) == nextBufferBarrier) {
                continue;
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
        }

        // collect all resources
        std::vector<std::tuple<Resource<Attachment>, RenderNode::AttachmentDesc>> attachments;
        for (const auto &[attachment, desc] : node.description.attachments.input) {
            attachments.emplace_back(std::make_tuple(attachment, desc));
        }

        for (const auto &[attachment, desc] : node.description.attachments.output) {
            attachments.emplace_back(std::make_tuple(attachment, desc));
        }

        for (const auto &[attachment, desc] : attachments) {
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
            if (node.description.attachments.input.contains(attachment)) {
                switch (desc.stage) {
                case RenderNode::Stage::Vertex:
                case RenderNode::Stage::Compute:
                    Logger::ERROR("[RenderGraph] Cannot use attachment in Vertex or Compute stage.");
                    return false;

                case RenderNode::Stage::Fragment:
                case RenderNode::Stage::Graphics:
                    switch (attachment->getInfo().type) {
                    case Attachment::Type::Color:
                        nextAccess |= vk::AccessFlagBits2::eInputAttachmentRead;
                        break;

                    case Attachment::Type::Depth:
                    case Attachment::Type::Stencil:
                    case Attachment::Type::DepthStencil:
                        nextAccess |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;
                        break;
                    }

                    switch (attachment->getInfo().type) {
                    case Attachment::Type::Color:
                        nextLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                        break;

                    case Attachment::Type::Depth:
                        nextLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                        break;

                    case Attachment::Type::Stencil:
                        nextLayout = vk::ImageLayout::eStencilReadOnlyOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                        break;

                    case Attachment::Type::DepthStencil:
                        nextLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eFragmentShader;
                        break;
                    }
                    break;

                case RenderNode::Stage::Transfer:
                    nextAccess |= vk::AccessFlagBits2::eTransferRead;
                    nextLayout = vk::ImageLayout::eTransferSrcOptimal;
                    break;
                }
            }

            // write attachment
            if (node.description.attachments.output.contains(attachment)) {
                switch (desc.stage) {
                case RenderNode::Stage::Vertex:
                case RenderNode::Stage::Compute:
                    Logger::ERROR("[RenderGraph] Cannot use attachment in Vertex or Compute stage.");
                    return false;

                case RenderNode::Stage::Graphics:
                case RenderNode::Stage::Fragment:
                    switch (attachment->getInfo().type) {
                    case Attachment::Type::Color:
                        nextAccess |= vk::AccessFlagBits2::eColorAttachmentWrite;
                        break;

                    case Attachment::Type::Depth:
                    case Attachment::Type::Stencil:
                    case Attachment::Type::DepthStencil:
                        nextAccess |= vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
                        break;
                    }

                    switch (attachment->getInfo().type) {
                    case Attachment::Type::Color:
                        nextLayout = vk::ImageLayout::eAttachmentOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                        break;

                    case Attachment::Type::Depth:
                        nextLayout = vk::ImageLayout::eDepthAttachmentOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests;
                        break;

                    case Attachment::Type::Stencil:
                        nextLayout = vk::ImageLayout::eStencilAttachmentOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests;
                        break;

                    case Attachment::Type::DepthStencil:
                        nextLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                        nextStage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests;
                        break;
                    }

                    break;

                case RenderNode::Stage::Transfer:
                    nextAccess |= vk::AccessFlagBits2::eTransferWrite;
                    nextLayout = vk::ImageLayout::eTransferDstOptimal;
                    break;
                }
            }

            nextAttachmentBarrier = {nextStage, nextAccess};

            // skip read-on-read hazards
            if (prevAttachmentBarrier.at(attachment) == nextAttachmentBarrier) {
                continue;
            }

            // store barrier
            barriers.m_AttachmentBarriers.emplace_back(std::make_tuple(
                vk::ImageMemoryBarrier2{
                    .srcStageMask = std::get<0>(prevAttachmentBarrier.at(attachment)),
                    .srcAccessMask = std::get<1>(prevAttachmentBarrier.at(attachment)),
                    .dstStageMask = std::get<0>(nextAttachmentBarrier),
                    .dstAccessMask = std::get<1>(nextAttachmentBarrier),
                    .oldLayout = prevAttachmentLayout[attachment],
                    .newLayout = nextLayout,
                },
                attachment));

            prevAttachmentBarrier[attachment] = nextAttachmentBarrier;
            prevAttachmentLayout[attachment] = nextLayout;
        }
    }

    return success;
}

void RenderGraph::execute(Scene *scene, const RenderContext &context) const {
    // prepare any nodes for rebuilding, validating, etc
    for (std::size_t i = 0; i < m_ExecutableNodeIds.size(); i++) {
        m_ExecutableNodes[i].prepare(scene, context);
    }

    // insert barriers and execute
    for (std::size_t i = 0; i < m_ExecutableNodeIds.size(); i++) {
        m_ExecutableNodeBarriers[i].apply(context);
        m_ExecutableNodes[i].execute(scene, context);
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
        elem.subresourceRange = attachment->getRingData()[context.frameInFlight].subresourceRange;
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
