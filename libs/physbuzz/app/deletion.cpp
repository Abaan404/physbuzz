#include "deletion.hpp"

#include "../render/buffer.hpp"
#include "../render/shaders.hpp"
#include <ranges>

namespace Physbuzz {

void DeletionQueue::enqueue(Buffer &&buffer) {
    m_Buffers.push_back(buffer);
}

void DeletionQueue::enqueue(RenderPipeline &&pipeline) {
    m_Pipelines.push_back(pipeline);
}

void DeletionQueue::flush() {
    for (auto buffer : std::views::reverse(m_Buffers)) {
        buffer.destroy();
    }

    m_Buffers.clear();

    for (auto buffer : std::views::reverse(m_Pipelines)) {
        buffer.destroy();
    }

    m_Pipelines.clear();
}

} // namespace Physbuzz
