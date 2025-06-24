#pragma once

#include "../../render/model.hpp"
#include "../../resources/handle.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ScreenQuad {

inline ResourceHandle<ModelResource> Resource = {"builtin/passthrough"};

bool build();

} // namespace ScreenQuad

} // namespace Builtin

} // namespace Physbuzz
