#include "models.hpp"

#include "../../ecs/scene.hpp"
#include "../../graphics/material.hpp"
#include "../defines.hpp"

namespace Physbuzz {

namespace Builtin {

RenderGraph RenderNodeModels::build(const std::unordered_set<ObjectID> &objects, std::vector<std::pair<Resource<Mesh>, std::size_t>> &batches) {
    RenderGraph graph = {{}};

    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBuffer)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBuffer,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(ModelBuffer));
    }

    if (!success) {
        Logger::ERROR("[RenderNodeModels] Failed to build buffers");
        return graph;
    }

    std::shared_ptr<std::vector<ModelBuffer>> data = std::make_shared<std::vector<ModelBuffer>>();

    graph.add(
        Id,
        {
            .description = {
                .buffers = {
                    .output = {
                        {
                            ResourceBuffer,
                            {
                                .stage = RenderNode::Stage::Transfer,
                            },
                        },
                    },
                },
            },
            .prepare = [&batches, &objects, data](Scene *scene, const RenderContext &context) {
                std::unordered_map<Resource<Mesh>, std::vector<ModelBuffer>> instances;

                std::size_t meshCount = 0;
                for (const auto &object : objects) {
                    const auto [render] = scene->getComponent<RenderComponent>(object);
                    context.materialAllocator->refresh(render.model, context);

                    const Model::Info &model = render.model.getInfo();
                    for (const auto &mesh : model.meshes) {
                        instances[mesh.mesh].emplace_back<ModelBuffer>({
                            .model = render.transform.matrix,
                            .normal = glm::transpose(glm::inverse(render.transform.matrix)),
                            .materialIdx = context.materialAllocator->query(mesh.material),
                        });
                    }

                    meshCount += render.model.getInfo().meshes.size();

                    batches.clear();
                    batches.reserve(instances.size());

                    data->clear();
                    data->reserve(meshCount);

                    for (const auto &[mesh, buffers] : instances) {
                        batches.emplace_back(std::make_tuple(mesh, buffers.size()));
                        data->insert(data->end(), std::make_move_iterator(buffers.begin()), std::make_move_iterator(buffers.end()));
                    }

                    std::size_t requiredSize = meshCount * sizeof(ModelBuffer);
                    if (ResourceBuffer->getSize(context.frameInFlight) < requiredSize) {
                        ResourceBuffer->rebuild(context, requiredSize);
                    }
                }
            },
            .execute = [data](Scene *scene, const RenderContext &context) {
                ResourceBuffer->update(context, *data);
            },
        });

    return graph;
}

} // namespace Builtin

} // namespace Physbuzz
