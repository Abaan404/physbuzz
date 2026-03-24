#include "renderstate.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../render/defines.hpp"
#include "descriptors/dynamic.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "pipeline.hpp"
#include <format>

namespace Physbuzz {

RenderState::RenderState(const Info &info)
    : m_Info(info),
      m_RenderGraph({}),
      m_Instance(std::format("{}instance", info.resourceIdPrefix)),
      m_Indirect(std::format("{}indirect", info.resourceIdPrefix)),
      m_Pipeline(std::format("{}pipeline", info.resourceIdPrefix)),
      m_Layout(std::format("{}layout", info.resourceIdPrefix)) {}

bool RenderState::build(const std::unordered_set<ObjectID> &objects) {
    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Instance)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_Instance,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(InstanceData));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Indirect)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            m_Indirect,
            {{
                .type = DynamicBuffer::Type::Indirect,
            }},
            sizeof(vk::DrawIndexedIndirectCommand));
    }

    if (!ResourceRegistry<DescriptorLayout>::contains(m_Layout)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            m_Layout,
            {{
                .bindings = {
                    {
                        // instance
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                    {
                        // indirect
                        .type = DescriptorLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(m_Pipeline)) {
        success &= ResourceRegistry<ComputePipeline>::insert(
            m_Pipeline,
            {
                {
                    .module = "builtin/culling/frustum",
                },
                {
                    .layouts = {
                        .resources = {
                            m_Layout,
                        },
                        .pushConstantRanges = {
                            {
                                .stageFlags = ComputePipeline::PushConstantsStageFlags::eCompute,
                                .size = sizeof(PushConstants),
                            },
                        },
                    },
                },
            });

        success &= App::LayoutAllocator.write(m_Layout, m_Instance, 0);
        success &= App::LayoutAllocator.write(m_Layout, m_Indirect, 1);
    }

    m_RenderGraph.add(
        std::format("{}setup", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .output = {
                        {
                            m_Instance,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                        {
                            m_Indirect,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .prepare = [this, &objects](Scene *scene, const RenderContext &context) {
                ZoneScopedN("RenderState/Node/Prepare");

                std::size_t instanceBufferSize = 0;

                // store a map of each mesh to the objects it corresponds to
                std::unordered_map<Resource<Mesh>, std::vector<ObjectID>> meshes;
                for (const auto &object : objects) {
                    const auto [render] = scene->getComponent<RenderComponent>(object);
                    const Model::Info &info = render.model.getInfo();

                    meshes[info.mesh].emplace_back(object);
                    instanceBufferSize += info.mesh->getInfo().submeshes.size();
                }

                m_InstanceBuffer.clear();
                m_InstanceBuffer.resize(instanceBufferSize);

                m_IndirectMeshes.clear();
                m_IndirectMeshes.reserve(meshes.size());

                m_IndirectBuffer.clear();
                m_IndirectBuffer.reserve(meshes.size());

                std::size_t meshOffset = 0;
                std::size_t indirectOffset = 0;

                // prepare buffers for execution
                for (const auto &[mesh, instances] : meshes) {
                    std::uint32_t instanceCount = instances.size();
                    std::size_t submeshCount = mesh->getInfo().submeshes.size();

                    // calculate each submeshes draw calls and store them
                    for (std::size_t submeshIdx = 0; submeshIdx < submeshCount; submeshIdx++) {
                        const Mesh::SubMesh &submesh = mesh->getInfo().submeshes[submeshIdx];

                        // use the firstInstance to lookup the correct instanceData in the buffer
                        std::uint32_t firstInstance = submeshIdx * instanceCount + meshOffset;

                        m_IndirectBuffer.emplace_back<vk::DrawIndexedIndirectCommand>({
                            .indexCount = submesh.indexCount,
                            .instanceCount = instanceCount,
                            .firstIndex = submesh.firstIndex,
                            .vertexOffset = submesh.vertexOffset,
                            .firstInstance = firstInstance,
                        });
                    }

                    // store as planar SoA so all index information can be stored in baseInstance during draw
                    for (std::size_t instanceIdx = 0; instanceIdx < instanceCount; instanceIdx++) {
                        const auto [render] = scene->getComponent<RenderComponent>(instances[instanceIdx]);
                        const Model::Info &info = render.model.getInfo();

                        for (std::size_t submeshIdx = 0; submeshIdx < submeshCount; submeshIdx++) {
                            const Mesh::SubMesh &submesh = mesh->getInfo().submeshes[submeshIdx];

                            // use the firstInstance to lookup the correct instanceData in the buffer
                            std::size_t firstInstance = submeshIdx * instanceCount + meshOffset;
                            std::size_t bufferIdx = instanceIdx + firstInstance;

                            m_InstanceBuffer[bufferIdx] = {
                                .model = render.transform.getModel(),
                                .normal = glm::transpose(glm::inverse(render.transform.getModel())),
                                .materialIdx = context.materialAllocator->query(info.materials[info.submeshMaterialIndices[submeshIdx]]),
                            };
                        }
                    }

                    // mesh has been evaluated, own this resource ref for execute()
                    m_IndirectMeshes.emplace_back(std::make_tuple(std::move(mesh), indirectOffset));

                    meshOffset += instanceCount * submeshCount;
                    indirectOffset += submeshCount;
                }

                std::size_t requiredInstanceSize = m_InstanceBuffer.size() * sizeof(InstanceData);
                if (m_Instance->getSize(context.frameInFlight) < requiredInstanceSize) {
                    m_Instance->rebuild(context, requiredInstanceSize);
                }

                std::size_t requiredIndirectSize = m_IndirectBuffer.size() * sizeof(vk::DrawIndexedIndirectCommand);
                if (m_Indirect->getSize(context.frameInFlight) < requiredIndirectSize) {
                    m_Indirect->rebuild(context, requiredIndirectSize);
                }
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("RenderNodeModels/Execute");
                TracyVkZone(context.tracy, context.command, "Model");

                m_Indirect->update(context, std::as_bytes(std::span(m_IndirectBuffer)), 0);
                m_Instance->update(context, m_InstanceBuffer);
            },
        });

    m_RenderGraph.add(
        std::format("{}output", m_Info.nodeIdPrefix),
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            m_Instance,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_Indirect,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                    .output = {
                        {
                            m_Instance,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                        {
                            m_Indirect,
                            {
                                .stage = RenderNode::Stage::Compute,
                            },
                        },
                    },
                },
            },
            .prepare = [this, &objects](Scene *scene, const RenderContext &context) {
                ZoneScopedN("RenderState/Cull/Prepare");
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("RenderState/Cull/Execute");
                TracyVkZone(context.tracy, context.command, "Model");

                m_Pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Pipeline);

                context.command.dispatch(m_IndirectBuffer.size(), 1, 1);
            },
        });

    return success;
}

bool RenderState::destroy() {
    bool success = true;

    success &= ResourceRegistry<ComputePipeline>::erase(m_Pipeline);
    success &= ResourceRegistry<DescriptorLayout>::erase(m_Layout);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_Instance);
    success &= ResourceRegistry<DynamicBuffer>::erase(m_Indirect);

    m_IndirectMeshes.clear();

    return success;
}

void RenderState::draw(const RenderContext &context) {
    ZoneScopedN("RenderState/Execute");
    TracyVkZone(context.tracy, context.command, "Model");

    for (const auto &[mesh, indirectOffset] : m_IndirectMeshes) {
        mesh->bind(context);

        context.command.drawIndexedIndirect(
            m_Indirect->getRingData()[context.frameInFlight].buffer.getData().buffer,
            indirectOffset * sizeof(vk::DrawIndexedIndirectCommand),
            mesh->getInfo().submeshes.size(),
            sizeof(vk::DrawIndexedIndirectCommand));
    }
}

const RenderGraph &RenderState::getGraph() const {
    return m_RenderGraph;
}

const RenderState::Info &RenderState::getInfo() const {
    return m_Info;
}

const Resource<DynamicBuffer> RenderState::getInstanceBuffer() const {
    return m_Instance;
}

const Resource<DynamicBuffer> RenderState::getIndirectBuffer() const {
    return m_Indirect;
}

} // namespace Physbuzz
