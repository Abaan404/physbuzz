#pragma once

#include "../../resources/defines.hpp"
#include "../buffer.hpp"
#include <vector>

namespace Physbuzz {

class StorageBuffer {
  public:
    template <typename T>
    struct Info {
        std::size_t count;
    };

    template <typename T>
    StorageBuffer(const Info<T> &info)
        : m_Stride(sizeof(T)),
          m_Count(info.count) {}

    bool build();
    bool destroy();

    template <typename T>
    bool update(std::uint32_t frameInFlight, const std::shared_ptr<Transfer> transfer, const std::vector<T> &data) const {
        std::span<const std::byte> bytes = {
            reinterpret_cast<const std::byte *>(data.data()),
            data.size() * sizeof(T),
        };

        PBZ_ASSERT(m_Buffers.size() == detail::MAX_FRAMES_IN_FLIGHT, "[StorageBuffer] Not built uniform.");
        PBZ_ASSERT(sizeof(T) == m_Stride, "[StorageBuffer] Invalid stride.");
        PBZ_ASSERT(data.size() <= m_Count, "[StorageBuffer] Invalid size.");

        return transfer->map(m_Buffers[frameInFlight], bytes);
    }

    const std::vector<Buffer> &getBuffers() const;
    std::size_t getRange() const;

  private:
    std::vector<Buffer> m_Buffers;

    std::size_t m_Stride;
    std::size_t m_Count;
};

template <>
struct IsResource<StorageBuffer> : std::true_type {};

} // namespace Physbuzz
