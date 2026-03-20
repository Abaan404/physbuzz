#include "deletion.hpp"

#include "application.hpp"
#include "imgui_impl_vulkan.h"
#include <ranges>

namespace Physbuzz {

void DeletionQueue::enqueue(Buffer &&buffer) {
    m_Buffers.push_back(buffer);
}

void DeletionQueue::enqueue(Image &&image) {
    m_Images.push_back(image);
}

void DeletionQueue::enqueue(Pipeline<GraphicsPipeline> &&pipeline) {
    m_GraphicsPipelines.push_back(pipeline);
}

void DeletionQueue::enqueue(vk::Sampler sampler) {
    m_Samplers.push_back(sampler);
}

void DeletionQueue::enqueue(vk::ImageView imageView) {
    m_ImageViews.push_back(imageView);
}

void DeletionQueue::enqueue(ImTextureID texId) {
    m_ImTextureIds.push_back(texId);
}

void DeletionQueue::flush() {
    for (auto &buffer : std::views::reverse(m_Buffers)) {
        buffer.destroy();
    }

    m_Buffers.clear();

    for (auto &imageView : std::views::reverse(m_ImageViews)) {
        App::Device.destroyImageView(imageView);
    }

    m_ImageViews.clear();

    for (auto &image : std::views::reverse(m_Images)) {
        image.destroy();
    }

    m_Images.clear();

    for (auto &pipeline : std::views::reverse(m_GraphicsPipelines)) {
        pipeline.destroy();
    }

    m_GraphicsPipelines.clear();

    for (auto &sampler : std::views::reverse(m_Samplers)) {
        App::Device.destroySampler(sampler);
    }

    m_Samplers.clear();

    for (auto &texId : std::views::reverse(m_ImTextureIds)) {
        ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(texId));
    }

    m_ImTextureIds.clear();
}

} // namespace Physbuzz
