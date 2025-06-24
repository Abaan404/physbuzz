#pragma once

#include "../../render/shaders.hpp"
#include "../../resources/handle.hpp"

namespace Physbuzz {

namespace Builtin {

namespace Passthrough {

inline ResourceHandle<ShaderPipelineResource> Resource = {"builtin/passthrough"};

bool build();

} // namespace Passthrough

namespace Depth {

inline ResourceHandle<ShaderPipelineResource> Resource = {"builtin/depth"};

bool build();

} // namespace Depth

} // namespace Builtin

} // namespace Physbuzz
