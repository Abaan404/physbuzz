#include "models.hpp"

#include "../../ecs/scene.hpp"
#include "../../graphics/material.hpp"
#include "../defines.hpp"
#include <memory>

namespace Physbuzz {

namespace Builtin {

RenderNode RenderNodeModels::build(Resource<DynamicBuffer> buffer, const std::unordered_set<ObjectID> &objects, std::vector<std::tuple<Resource<Mesh>, std::size_t>> &batches) {
    std::shared_ptr<std::vector<ModelBuffer>> data = std::make_shared<std::vector<ModelBuffer>>();

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
        .prepare = [&objects, &batches, buffer, data](Scene *scene, const RenderContext &context) {
            std::unordered_map<Resource<Mesh>, std::vector<ModelBuffer>> instances;

            std::size_t meshCount = 0;
            for (const auto &object : objects) {
                const auto [render] = scene->getComponent<RenderComponent>(object);

                const Model::Info &model = render.model.getInfo();
                for (const auto &mesh : model.meshes) {
                    instances[mesh.mesh].emplace_back<ModelBuffer>({
                        .model = render.transform.getModel(),
                        .normal = glm::transpose(glm::inverse(render.transform.getModel())),
                        .materialIdx = context.materialAllocator->query(mesh.material),
                    });
                }

                meshCount += model.meshes.size();
            }

            batches.clear();
            batches.reserve(instances.size());

            data->clear();
            data->reserve(meshCount);

            for (const auto &[mesh, buffers] : instances) {
                batches.emplace_back(std::make_tuple(mesh, buffers.size()));
                data->insert(data->end(), std::make_move_iterator(buffers.begin()), std::make_move_iterator(buffers.end()));
            }

            std::size_t requiredSize = meshCount * sizeof(ModelBuffer);
            if (buffer->getSize(context.frameInFlight) < requiredSize) {
                buffer->rebuild(context, requiredSize);
            }
        },
        .execute = [buffer, data](Scene *scene, const RenderContext &context) {
            TracyVkZone(context.tracy, context.command, "Model");

            buffer->update(context, *data);
        },
    };
}

} // namespace Builtin

} // namespace Physbuzz
