#pragma once

#include "../../events/handler.hpp"
#include "../../resources/defines.hpp"
#include "../memory.hpp"

namespace Physbuzz {

struct RenderContext;

class StaticBuffer : public EventSubject {
  public:
    struct Data {
        Buffer buffer = {{}};
        vk::DeviceAddress address = 0;
    };

    StaticBuffer();

    bool build(std::uint64_t size);
    bool destroy();

    bool rebuild(const RenderContext &context, std::uint64_t size);

    template <typename T>
    bool update(const RenderContext &context, const std::vector<T> &data, std::uint32_t index = 0) {
        std::span<const std::byte> bytes = std::as_bytes(std::span(data));
        return update(context, bytes, sizeof(T) * index);
    }

    bool update(const RenderContext &context, const std::span<const std::byte> &bytes, std::uint64_t offset);

    std::size_t getSize() const;
    const Data &getData() const;

  private:
    Data m_Data;
};

template <>
struct IsResource<StaticBuffer> : std::true_type {};

} // namespace Physbuzz
