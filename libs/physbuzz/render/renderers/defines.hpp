#pragma once

#include "../../resources/resources.hpp"
#include "../transform.hpp"
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

namespace detail {

static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

}

class Model;

struct RenderComponent {
    Transform transform;
    Resource<Model> model;
};

class IRenderPass {
  public:
    virtual ~IRenderPass() = default;

    virtual void render(const vk::CommandBuffer &commandBuffer, std::uint32_t frameInFlight) = 0;
};

} // namespace Physbuzz
