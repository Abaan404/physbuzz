#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

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

    vk::CommandBuffer command;
    vk::Extent2D extent;
    std::uint32_t frameInFlight;

    struct {
        vk::Image image;
        vk::ImageView view;
    } color;

    struct {
        std::shared_ptr<Transfer> transfer;
        std::shared_ptr<PipelineLayoutAllocator> allocator;
    } systems;
};

} // namespace Physbuzz
