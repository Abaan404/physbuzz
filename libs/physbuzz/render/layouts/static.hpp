#pragma once

#include "../../resources/defines.hpp"
#include "../buffer.hpp"
#include "../renderer.hpp"
#include <span>
#include <vector>

namespace Physbuzz {

class StaticBuffer {
  public:
    enum class Type {
        Constant,
        Structured,
        ConstantDynamic,
        StructuredDynamic,
    };

    template <typename T>
    struct Info {
        Type type;
        std::size_t count = 1;
    };

    template <typename T>
    StaticBuffer(const Info<T> &info)
        : m_Type(info.type),
          m_Stride(sizeof(T)),
          m_Count(info.count) {}

    bool build();
    bool destroy();

    template <typename T>
    bool update(const std::shared_ptr<Renderer> renderer, const std::shared_ptr<Transfer> transfer, const std::vector<T> &data, std::uint32_t index = 0) const {
        PBZ_ASSERT(sizeof(T) == m_Stride, "[StaticBuffer] Invalid stride.");
        PBZ_ASSERT(data.size() <= m_Count, "[StaticBuffer] Invalid size.");

        std::span<const std::byte> bytes = {
            reinterpret_cast<const std::byte *>(data.data()),
            data.size() * sizeof(T),
        };

        return update(renderer, transfer, bytes, bytes.size() * index);
    }

    bool update(const std::shared_ptr<Renderer> renderer, const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const;

    const std::vector<Buffer> &getBuffers() const;
    std::uint64_t getSize() const;
    std::uint64_t getStride() const;
    Type getType() const;

  private:
    std::vector<Buffer> m_Buffers;

    Type m_Type;
    std::uint64_t m_Stride;
    std::uint32_t m_Count;
};

template <>
struct IsResource<StaticBuffer> : std::true_type {};

} // namespace Physbuzz
