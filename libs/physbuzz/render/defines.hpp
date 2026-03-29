#pragma once

#include "../graphics/model.hpp"
#include "../math/transform.hpp"

namespace Physbuzz {

struct RenderComponent {
    Transform transform;
    Model model;
};

} // namespace Physbuzz
