#pragma once

#include "../app/deletion.hpp"
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class PipelineLayoutAllocator;

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

    struct {
        std::shared_ptr<Transfer> transfer;
        std::shared_ptr<PipelineLayoutAllocator> allocator;
    } systems;
};

class IRenderPass {
  public:
    virtual ~IRenderPass() = default;

    virtual void render(const RenderContext &context) = 0;
};

} // namespace Physbuzz
