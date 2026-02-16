#pragma once

#include "../../ecs/defines.hpp"
#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

namespace Builtin {

namespace RenderNodeCamera {

struct CameraBuffer {
    alignas(16) glm::vec3 position;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

inline Resource<DynamicBuffer> ResourceBuffer = {"builtin/camera"};

inline RenderNodeID Id = "builtin/camera";

RenderNode build(const ObjectID &object);

} // namespace RenderNodeCamera

} // namespace Builtin

} // namespace Physbuzz
