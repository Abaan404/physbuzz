#pragma once

#include "../graphics/memory.hpp"
#include "../graphics/pipeline.hpp"
#include <imgui.h>

namespace Physbuzz {

class DeletionQueue {
  public:
    // engine types
    void enqueue(Buffer &&buffer);
    void enqueue(Image &&image);
    void enqueue(RenderPipeline &&pipeline);

    // vk handles
    void enqueue(vk::Sampler sampler);
    void enqueue(vk::ImageView imageView);

    // imgui handles
    void enqueue(ImTextureID texId);

    void flush();

  private:
    std::vector<Buffer> m_Buffers;
    std::vector<Image> m_Images;
    std::vector<RenderPipeline> m_Pipelines;

    std::vector<vk::Sampler> m_Samplers;
    std::vector<vk::ImageView> m_ImageViews;

    std::vector<ImTextureID> m_ImTextureIds;
};

} // namespace Physbuzz
