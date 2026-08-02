#pragma once

#include <vulkan/vulkan.hpp>

#include <tracy/TracyVulkan.hpp>

namespace Physbuzz {

class GraphicsPipeline;
class ComputePipeline;
class DescriptorLayoutAllocator;
class MaterialAllocator;
class DeletionQueue;
class Transfer;
class Attachment;

namespace detail {

static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

}

struct RenderContext {
    MaterialAllocator *materialAllocator;
    TracyVkCtx tracy;

    DeletionQueue *deletionQueue;
    vk::CommandBuffer command;
    std::uint32_t frameInFlight;

    vk::Extent2D extent;
    const Attachment *depth;
    struct {
        vk::Image image;
        vk::ImageView view;
    } color;
};

template <typename T>
concept PipelineType =
    std::same_as<T, GraphicsPipeline> ||
    std::same_as<T, ComputePipeline>;

} // namespace Physbuzz
