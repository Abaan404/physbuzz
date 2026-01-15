#pragma once

#include "../app/deletion.hpp"
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

namespace detail {

static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

}

struct RenderContext {
    DeletionQueue *deletionQueue;

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

} // namespace Physbuzz
