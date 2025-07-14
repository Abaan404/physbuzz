#pragma once

#include "../../render/model.hpp"
#include "../../resources/resources.hpp"

namespace Physbuzz {

namespace Builtin {

namespace MeshScreenQuad {

inline Resource<Model> Resource = {"builtin/screenquad"};

bool build();

} // namespace ScreenQuad

} // namespace Builtin

} // namespace Physbuzz
