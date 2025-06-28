#pragma once

#include "../../render/shaders.hpp"
#include "../../resources/handle.hpp"

namespace Physbuzz {

namespace Builtin {

namespace Passthrough {

inline ResourceHandle<ShaderPipelineResource> Resource = {"builtin/passthrough"};

bool build();

} // namespace Passthrough

namespace Depth2D {

inline ResourceHandle<ShaderPipelineResource> Resource = {"builtin/depth/2D"};

bool build();

} // namespace Depth

namespace DepthCubemap {

inline ResourceHandle<ShaderPipelineResource> Resource = {"builtin/depth/cubemap"};

bool build();

} // namespace Depth

} // namespace Builtin

} // namespace Physbuzz
