#pragma once

#include "../../ecs/defines.hpp"
#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

class Mesh;

namespace Builtin {

namespace RenderNodeModels {

struct ModelBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 normal;
    std::uint32_t materialIdx;
};

RenderNode build(Resource<DynamicBuffer> buffer, const std::unordered_set<ObjectID> &objects, std::vector<std::tuple<Resource<Mesh>, std::size_t>> &batches);

} // namespace RenderNodeModels

} // namespace Builtin

} // namespace Physbuzz
