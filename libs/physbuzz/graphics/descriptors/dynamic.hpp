#pragma once

#include "../../resources/defines.hpp"
#include "../defines.hpp"
#include <variant>

namespace Physbuzz {

class DynamicBuffer {
  public:
    enum class Type {
        Constant,
        Structured,
        ConstantDynamic,
        StructuredDynamic,
    };

    struct Info {
        Type type;
    };

    DynamicBuffer(const Info &info);

    bool build(std::uint64_t size);
    bool destroy();

    bool resize(const RenderContext &context, std::uint64_t size);

    template <typename T>
    bool update(const RenderContext &context, const std::shared_ptr<Transfer> transfer, const std::vector<T> &data, std::uint32_t index = 0) const {
        std::span<const std::byte> bytes = std::as_bytes(std::span(data));
        return update(context, transfer, bytes, sizeof(T) * index);
    }

    bool update(const RenderContext &context, const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const;

    const Info &getInfo() const;

    std::size_t getSize(const RenderContext &context) const;
    const std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT> &getBuffers() const;

  private:
    Info m_Info;

    std::variant<std::monostate, std::array<Buffer, detail::MAX_FRAMES_IN_FLIGHT>> m_Buffers;
};

template <>
struct IsResource<DynamicBuffer> : std::true_type {};

} // namespace Physbuzz
