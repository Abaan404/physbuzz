#pragma once

#include "../graphics/memory.hpp"
#include "../graphics/pipeline.hpp"

namespace Physbuzz {

class Buffer;
class RenderPipeline;

class DeletionQueue {
  public:
    // engine types
    void enqueue(Buffer &&buffer);
    void enqueue(Image &&image);
    void enqueue(RenderPipeline &&pipeline);

    // vk handles
    void enqueue(vk::Sampler sampler);
    void enqueue(vk::ImageView imageView);

    void flush();

  private:
    std::vector<Buffer> m_Buffers;
    std::vector<Image> m_Images;
    std::vector<RenderPipeline> m_Pipelines;

    std::vector<vk::Sampler> m_Samplers;
    std::vector<vk::ImageView> m_ImageViews;
};

} // namespace Physbuzz
