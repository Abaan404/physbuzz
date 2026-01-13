#pragma once

#include "../../resources/defines.hpp"
#include "../buffer.hpp"
#include "../renderers/defines.hpp"
#include "defines.hpp"

namespace Physbuzz {

class ShaderBuffer {
  public:
    enum class Type {
        Constant,
        Structured,
        ConstantDynamic,
        StructuredDynamic,
    };

    struct Info {
        Type type;
        LayoutLifetime lifetime = LayoutLifetime::PerFrame;
    };

    ShaderBuffer(const Info &info);

    bool build(std::uint64_t size);
    bool destroy();

    bool resize(RenderContext context, std::uint64_t size);

    template <typename T>
    bool update(const RenderContext &context, const std::shared_ptr<Transfer> transfer, const std::vector<T> &data, std::uint32_t index = 0) const {
        std::span<const std::byte> bytes = {
            reinterpret_cast<const std::byte *>(data.data()),
            data.size() * sizeof(T),
        };

        return update(context, transfer, bytes, sizeof(T) * index);
    }

    bool update(const RenderContext &context, const std::shared_ptr<Transfer> transfer, const std::span<const std::byte> &bytes, std::uint64_t offset) const;

    const Info &getInfo() const;
    const std::vector<Buffer> &getBuffers() const;

  private:
    Info m_Info;

    std::vector<Buffer> m_Buffers;
};

template <>
struct IsResource<ShaderBuffer> : std::true_type {};

} // namespace Physbuzz
