#pragma once

#include "../../render/model.hpp"
#include "../../resources/resources.hpp"

namespace Physbuzz {

namespace Builtin {

namespace ScreenQuad {

inline Resource<Model> Resource = {"builtin/passthrough"};

bool build();

} // namespace ScreenQuad

} // namespace Builtin

} // namespace Physbuzz
