#pragma once

#include "../../events/handler.hpp"
#include "../../resources/defines.hpp"
#include "../defines.hpp"
#include "../memory.hpp"
#include <variant>

namespace Physbuzz {

class DynamicBuffer : public EventSubject {
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

    struct Data {
        Buffer buffer;
    };

    DynamicBuffer(const Info &info);

    bool build(std::uint64_t size);
    bool destroy();

    bool rebuild(const RenderContext &context, std::uint64_t size);

    template <typename T>
    bool update(const RenderContext &context, const std::vector<T> &data, std::uint32_t index = 0) {
        std::span<const std::byte> bytes = std::as_bytes(std::span(data));
        return update(context, bytes, sizeof(T) * index);
    }

    bool update(const RenderContext &context, const std::span<const std::byte> &bytes, std::uint64_t offset);

    const Info &getInfo() const;
    const std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &getRingData() const;

    std::size_t getSize(std::uint32_t frameInFlight) const;

  private:
    Info m_Info;

    std::variant<std::monostate, std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>> m_RingData;
};

template <>
struct IsResource<DynamicBuffer> : std::true_type {};

} // namespace Physbuzz
