#pragma once

#include "../../resources/defines.hpp"
#include "../buffer.hpp"
#include "../renderers/defines.hpp"
#include <span>
#include <vector>

namespace Physbuzz {

class ShaderBuffer {
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
    };

    template <typename T>
    ShaderBuffer(const Info<T> &info)
        : m_Type(info.type),
          m_Stride(sizeof(T)) {}

    bool build(std::uint64_t count = 1);
    bool destroy();

    bool resize(RenderContext context, std::uint64_t count);

    template <typename T>
    bool update(const RenderContext &context, const std::shared_ptr<Transfer> transfer, const std::vector<T> &data, std::uint32_t index = 0) const {
        std::span<const std::byte> bytes = {
            reinterpret_cast<const std::byte *>(data.data()),
            data.size() * sizeof(T),
        };

        return update(context, transfer, bytes, bytes.size() * index);
    }

    bool update(const RenderContext &context, const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const;

    const std::vector<Buffer> &getBuffers() const;
    std::uint64_t getStride() const;
    Type getType() const;

  private:
    std::vector<Buffer> m_Buffers;

    Type m_Type;
    std::uint64_t m_Stride;
};

template <>
struct IsResource<ShaderBuffer> : std::true_type {};

} // namespace Physbuzz
