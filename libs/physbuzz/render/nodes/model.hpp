#pragma once

#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

class Mesh;

namespace Builtin {

namespace RenderNodeModels {

struct ModelBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 invModel;
    std::uint32_t materialIdx;
};

inline Resource<DynamicBuffer> ResourceBuffer = {"builtin/model"};

RenderNode build(const std::unordered_set<ObjectID> &objects, std::vector<std::pair<Resource<Mesh>, std::size_t>> &meshes);

} // namespace RenderNodeModels

} // namespace Builtin

} // namespace Physbuzz
