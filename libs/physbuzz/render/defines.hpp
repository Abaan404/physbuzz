#pragma once

#include "model.hpp"
#include "transform.hpp"

namespace Physbuzz {

struct RenderComponent {
    Transform transform;
    Model model;
};

class IRenderPass {
  public:
    virtual ~IRenderPass() = default;

    virtual void render(const RenderContext &context) = 0;
};

} // namespace Physbuzz
