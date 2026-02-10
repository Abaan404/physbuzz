#include "model.hpp"

#include "../../ecs/scene.hpp"
#include "../defines.hpp"
#include "physbuzz/graphics/material.hpp"
#include "physbuzz/graphics/mesh.hpp"

namespace Physbuzz {

namespace Builtin {

RenderNode RenderNodeModels::build(const std::unordered_set<ObjectID> &objects, std::vector<std::pair<Resource<Mesh>, std::size_t>> &batches) {
    return {
        .description = {
            .buffers = {
                .output = {
                    {
                        ResourceBuffer,
                        {
                            {
                                .type = DynamicBuffer::Type::Structured,
                            },
                            sizeof(ModelBuffer),
                        },
                    },
                },
            },
        },
        .execute = [&batches, &objects](Scene *scene, const RenderContext &context) {
            std::unordered_map<Resource<Mesh>, std::vector<ModelBuffer>> instances;
            batches.clear();

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
            }

            std::vector<ModelBuffer> instanceBuffers;

            instanceBuffers.reserve(meshCount);
            batches.reserve(instances.size());

            for (const auto &[mesh, buffers] : instances) {
                instanceBuffers.insert(instanceBuffers.end(), std::make_move_iterator(buffers.begin()), std::make_move_iterator(buffers.end()));
                batches.emplace_back<std::tuple<Resource<Mesh>, std::size_t>>({mesh, buffers.size()});
            }

            ResourceBuffer->update(context, instanceBuffers);
        },
    };
}

} // namespace Builtin

} // namespace Physbuzz
