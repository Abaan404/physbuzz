#pragma once

#include "../../resources/defines.hpp"
#include "../buffer.hpp"
#include "../renderer.hpp"
#include "../renderers/defines.hpp"
#include <vector>

namespace Physbuzz {

class StaticBuffer {
  public:
    enum class Type {
        Constant,
        Structured,
    };

    template <typename T>
    struct Info {
        std::size_t count;
        Type type;
    };

    template <typename T>
    StaticBuffer(const Info<T> &info)
        : m_Type(info.type),
          m_Stride(sizeof(T)),
          m_Count(info.count) {}

    bool build();
    bool destroy();

    template <typename T>
    bool update(const std::shared_ptr<Renderer> renderer, const std::shared_ptr<Transfer> transfer, const std::vector<T> &data) const {
        std::span<const std::byte> bytes = {
            reinterpret_cast<const std::byte *>(data.data()),
            data.size() * sizeof(T),
        };

        PBZ_ASSERT(m_Buffers.size() == detail::MAX_FRAMES_IN_FLIGHT, "[StaticBuffer] Not built uniform.");
        PBZ_ASSERT(sizeof(T) == m_Stride, "[StaticBuffer] Invalid stride.");
        PBZ_ASSERT(data.size() <= m_Count, "[StaticBuffer] Invalid size.");

        return transfer->map(m_Buffers[renderer->getFrameInFlight()], bytes);
    }

    const std::vector<Buffer> &getBuffers() const;
    std::size_t getRange() const;
    Type getType() const;

  private:
    std::vector<Buffer> m_Buffers;

    Type m_Type;
    std::size_t m_Stride = 0;
    std::size_t m_Count = 0;
};

template <>
struct IsResource<StaticBuffer> : std::true_type {};

} // namespace Physbuzz
