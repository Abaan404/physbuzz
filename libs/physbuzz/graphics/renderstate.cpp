#include "renderstate.hpp"

#include "../ecs/scene.hpp"
#include "../render/defines.hpp"
#include "descriptors/dynamic.hpp"
#include "material.hpp"
#include "model.hpp"

namespace Physbuzz {

RenderState::RenderState(const Info &info)
    : m_Info(info), m_Instance(info.instanceBufferId), m_Indirect(info.indirectBufferId) {}

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

    m_RenderNode = {
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
    };

    return success;
}

bool RenderState::destroy() {
    bool success = true;

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

const RenderNode &RenderState::getRenderNode() const {
    return m_RenderNode;
}

const RenderState::Info &RenderState::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
