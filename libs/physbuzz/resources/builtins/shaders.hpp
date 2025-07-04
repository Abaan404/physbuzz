#pragma once

#include "../../render/shaders.hpp"
#include "../../resources/resources.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ShaderPassthrough {

inline Resource<ShaderPipeline> Resource = {"builtin/passthrough"};

bool build();

} // namespace Passthrough

namespace ShaderDepth2D {

inline Resource<ShaderPipeline> Resource = {"builtin/depth/2D"};

bool build();

} // namespace Depth

namespace ShaderDepthCubemap {

inline Resource<ShaderPipeline> Resource = {"builtin/depth/cubemap"};

bool build();

} // namespace Depth

} // namespace Builtin

} // namespace Physbuzz
