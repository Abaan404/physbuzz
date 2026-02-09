#pragma once

#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

namespace Builtin {

namespace RenderNodeLights {

inline Resource<DynamicBuffer> ResourceBufferDirectional = {"builtin/lights/directional"};
inline Resource<DynamicBuffer> ResourceBufferPoint = {"builtin/lights/point"};
inline Resource<DynamicBuffer> ResourceBufferSpot = {"builtin/lights/spot"};

RenderNode build();

} // namespace RenderNodeLights

} // namespace Builtin

} // namespace Physbuzz
