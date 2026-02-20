#pragma once

#include "../../events/handler.hpp"
#include "../../resources/defines.hpp"
#include "../defines.hpp"
#include "../memory.hpp"

namespace Physbuzz {

class Attachment : public EventSubject {
  public:
    using Format = vk::Format;

    enum class Type {
        Color,
        Depth,
        Stencil,
        DepthStencil,
    };

    struct Info {
        Type type = Type::Color;
        Format format = Format::eR8G8B8A8Unorm;
    };

    struct Data {
        Image image;

        vk::ImageView view = nullptr;
        vk::ImageSubresourceRange subresourceRange = {};
    };

    Attachment(const Info &info);

    bool build(const glm::uvec2 &resolution);
    bool destroy();

    bool rebuild(const RenderContext &context, const glm::uvec2 &size);

    const Info &getInfo() const;
    const std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &getRingData() const;

    glm::uvec2 getSize(std::uint32_t frameInFlight) const;

  private:
    std::tuple<vk::ImageView, vk::ImageSubresourceRange> createImageView(const Image &image) const;
    vk::ImageSubresourceRange createSubresourceRange(const Image &image) const;

    Info m_Info;

    std::variant<std::monostate, std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>> m_RingData;
};

template <>
struct IsResource<Attachment> : std::true_type {};

} // namespace Physbuzz
