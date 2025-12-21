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

struct RenderContext {
    vk::CommandBuffer command;
    vk::Extent2D extent;
    std::uint32_t frameInFlight;

    struct {
        vk::Image image;
        vk::ImageView view;
    } color;

    struct {
        vk::Image image;
        vk::ImageView view;
    } depth;

};

class IRenderPass {
  public:
    virtual ~IRenderPass() = default;

    virtual void render(const RenderContext &context) = 0;
};

} // namespace Physbuzz
