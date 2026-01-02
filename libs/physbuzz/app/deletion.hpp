#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Buffer;

class DeletionQueue {
  public:
    void enqueue(const Buffer &buffer);

    void flush();

  private:
    std::vector<Buffer> m_Buffers;
};

} // namespace Physbuzz
