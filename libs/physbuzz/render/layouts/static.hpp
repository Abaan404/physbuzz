#pragma once

#include "../../resources/defines.hpp"
#include "../buffer.hpp"

namespace Physbuzz {

struct RenderContext;

class StaticBuffer {
  public:
    StaticBuffer();

    bool build(std::uint64_t size);
    bool destroy();

    bool resize(const RenderContext &context, const std::shared_ptr<Transfer> transfer, std::uint64_t size);

    template <typename T>
    bool update(const std::shared_ptr<Transfer> transfer, const std::vector<T> &data, std::uint32_t index = 0) const {
        std::span<const std::byte> bytes = std::as_bytes(std::span(data));
        return update(transfer, bytes, sizeof(T) * index);
    }

    bool update(const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const;

    const Buffer &getBuffer() const;
    const vk::DeviceAddress &getAddress() const;

  private:
    Buffer m_Buffer;
    vk::DeviceAddress m_Address = 0;
};

template <>
struct IsResource<StaticBuffer> : std::true_type {};

} // namespace Physbuzz
