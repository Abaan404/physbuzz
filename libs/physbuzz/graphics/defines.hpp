#pragma once

#include <vulkan/vulkan.hpp>

#include <tracy/TracyVulkan.hpp>

namespace Physbuzz {

class PipelineLayoutAllocator;
class MaterialAllocator;
class DeletionQueue;
class Transfer;
class Attachment;

namespace detail {

static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

}

struct RenderContext {
    DeletionQueue *deletionQueue;
    MaterialAllocator *materialAllocator;

    const Attachment *depth;
    TracyVkCtx tracy;

    vk::CommandBuffer command;
    vk::Extent2D extent;
    std::uint32_t frameInFlight;

    struct {
        vk::Image image;
        vk::ImageView view;
    } color;
};

} // namespace Physbuzz
