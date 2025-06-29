#pragma once

#include "../../render/shaders.hpp"
#include "../../resources/resources.hpp"

namespace Physbuzz {

namespace Builtin {

namespace Passthrough {

inline Resource<ShaderPipeline> Resource = {"builtin/passthrough"};

bool build();

} // namespace Passthrough

namespace Depth2D {

inline Resource<ShaderPipeline> Resource = {"builtin/depth/2D"};

bool build();

} // namespace Depth

namespace DepthCubemap {

inline Resource<ShaderPipeline> Resource = {"builtin/depth/cubemap"};

bool build();

} // namespace Depth

} // namespace Builtin

} // namespace Physbuzz
