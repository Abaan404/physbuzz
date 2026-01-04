#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Buffer;
class RenderPipeline;

class DeletionQueue {
  public:
    void enqueue(const Buffer &buffer);
    void enqueue(const RenderPipeline &pipeline);

    void flush();

  private:
    std::vector<Buffer> m_Buffers;
    std::vector<RenderPipeline> m_Pipelines;
};

} // namespace Physbuzz
