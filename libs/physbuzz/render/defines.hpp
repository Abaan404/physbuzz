#pragma once

#include "../graphics/model.hpp"
#include "../math/transform.hpp"

namespace Physbuzz {

struct RenderComponent {
    Model model;
};

struct TransformComponent {
    Transform transform;
};

} // namespace Physbuzz
