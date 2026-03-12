#include "models.hpp"

#include "../../ecs/scene.hpp"
#include "../../graphics/material.hpp"
#include "../defines.hpp"
#include <memory>
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

RenderNode RenderNodeModels::build(Resource<DynamicBuffer> buffer, const std::unordered_set<ObjectID> &objects, std::vector<std::tuple<Resource<Mesh>, std::size_t>> &batches) {
    std::shared_ptr<std::vector<ModelBuffer>> meshObjects = std::make_shared<std::vector<ModelBuffer>>();

    return {
        .description = {
            .buffers = {
                .output = {
                    {
                        buffer,
                        {
                            .stage = RenderNode::Stage::Transfer,
                        },
                    },
                },
            },
        },
        .prepare = [&objects, &batches, buffer, meshObjects](Scene *scene, const RenderContext &context) {
            ZoneScopedN("RenderNodeModels/Prepare");

            std::size_t bufferSize = 0;

            // store a map of each mesh to the objects it corresponds to
            std::unordered_map<Resource<Mesh>, std::vector<ObjectID>> meshes;
            for (const auto &object : objects) {
                const auto [render] = scene->getComponent<RenderComponent>(object);
                const Model::Info &info = render.model.getInfo();

                meshes[info.mesh].push_back(object);
                bufferSize += info.mesh->getInfo().submeshes.size();
            }

            meshObjects->clear();
            meshObjects->resize(bufferSize);

            std::size_t meshOffset = 0;

            // create object buffers based on the instances of objects it gathered
            for (const auto &[mesh, instances] : meshes) {
                std::size_t instanceCount = instances.size();
                std::size_t submeshCount = mesh->getInfo().submeshes.size();

                // store as planar SoA so all index information can be stored in baseInstance during draw
                for (std::size_t instanceId = 0; instanceId < instanceCount; instanceId++) {
                    const auto [render] = scene->getComponent<RenderComponent>(instances[instanceId]);
                    const Model::Info &info = render.model.getInfo();

                    for (std::size_t submeshId = 0; submeshId < submeshCount; submeshId++) {
                        // this calc is expected to be recomputed in shaders and draw calls
                        std::size_t objectId = instanceId + submeshId * instanceCount + meshOffset;

                        (*meshObjects)[objectId] = {
                            .model = render.transform.getModel(),
                            .normal = glm::transpose(glm::inverse(render.transform.getModel())),
                            .materialIdx = context.materialAllocator->query(info.materials[info.submeshMaterialIndices[submeshId]]),
                        };
                    }
                }

                meshOffset += instanceCount * submeshCount;
            }

            batches.clear();
            batches.reserve(meshes.size());

            // store the instanceCounts to calculate batchSize during draw calls (and also instances)
            for (const auto &[mesh, objects] : meshes) {
                batches.emplace_back(std::make_tuple(mesh, objects.size()));
            }

            std::size_t requiredSize = bufferSize * sizeof(ModelBuffer);
            if (buffer->getSize(context.frameInFlight) < requiredSize) {
                buffer->rebuild(context, requiredSize);
            }
        },
        .execute = [buffer, meshObjects](Scene *scene, const RenderContext &context) {
            ZoneScopedN("RenderNodeModels/Execute");
            TracyVkZone(context.tracy, context.command, "Model");

            buffer->update(context, *meshObjects);
        },
    };
}

} // namespace Builtin

} // namespace Physbuzz
