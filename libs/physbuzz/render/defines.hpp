#pragma once

#include "../graphics/model.hpp"
#include "../math/transform.hpp"

namespace Physbuzz {

struct Renderable {
    Model model;
    Transform transform;
};

struct RenderComponent {
    Renderable renderable;
};

} // namespace Physbuzz
