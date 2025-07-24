#pragma once

#include "../model.hpp"
#include "../transform.hpp"

namespace Physbuzz {

class Framebuffer;

class IRenderer {
  public:
    virtual ~IRenderer() = default;

    virtual void resize(const glm::ivec2 &resolution) = 0;
    virtual const Framebuffer &getFramebuffer() const = 0;
};

struct RenderComponent {
    Transform transform;
    Resource<Model> model;
};

} // namespace Physbuzz
