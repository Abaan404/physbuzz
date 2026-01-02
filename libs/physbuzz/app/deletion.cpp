#include "deletion.hpp"

#include "../render/buffer.hpp"
#include <ranges>

namespace Physbuzz {

void DeletionQueue::enqueue(const Buffer &buffer) {
    m_Buffers.push_back(buffer);
}

void DeletionQueue::flush() {
    for (auto buffer : std::views::reverse(m_Buffers)) {
        buffer.destroy();
    }

    m_Buffers.clear();
}

} // namespace Physbuzz
