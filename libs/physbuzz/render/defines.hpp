#pragma once

#include "model.hpp"
#include "transform.hpp"

namespace Physbuzz {

struct RenderComponent {
    Transform transform;
    Model model;
};

} // namespace Physbuzz
