#pragma once

#include "../render/buffer.hpp"
#include "../render/shaders.hpp"

namespace Physbuzz {

class Buffer;
class RenderPipeline;

class DeletionQueue {
  public:
    void enqueue(Buffer &&buffer);
    void enqueue(RenderPipeline &&pipeline);

    void flush();

  private:
    std::vector<Buffer> m_Buffers;
    std::vector<RenderPipeline> m_Pipelines;
};

} // namespace Physbuzz
