#pragma once

#include "../../resources/resources.hpp"
#include "../transform.hpp"
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

namespace detail {

static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

}

class Model;
class Framebuffer;

class IRenderer {
  public:
    virtual ~IRenderer() = default;

    virtual void resize(const glm::ivec2 &resolution) = 0;
    virtual const Framebuffer &getOutput() const = 0;
};

struct RenderComponent {
    Transform transform;
    Resource<Model> model;
};

} // namespace Physbuzz
