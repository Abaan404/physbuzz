#pragma once

#include "../graphics/model.hpp"
#include "../graphics/transform.hpp"

namespace Physbuzz {

struct RenderComponent {
    Transform transform;
    Model model;
};

} // namespace Physbuzz
