#pragma once

#include "../../events/handler.hpp"
#include "../../resources/defines.hpp"
#include "../defines.hpp"
#include "../memory.hpp"
#include "sampler.hpp"

namespace Physbuzz {

class Attachment : public EventSubject {
  public:
    using Format = vk::Format;

    enum class Type {
        Dim2D,
        Cube,
    };

    enum class Usage {
        Color,
        Depth,
        Stencil,
        DepthStencil,
    };

    struct Info {
        Type type = Type::Dim2D;
        Usage usage = Usage::Color;
        Format format = Format::eR16G16B16A16Sfloat;

        Sampler sampler = {{Sampler::Type::None}};
        std::vector<Image::ViewInfo> views = {};
    };

    struct Data {
        Image image;
    };

    Attachment(const Info &info);

    bool build(const glm::uvec2 &resolution);
    bool destroy();

    bool rebuild(const RenderContext &context, const glm::uvec2 &size);

    const Info &getInfo() const;
    const std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &getRingData() const;

    glm::uvec2 getSize(std::uint32_t frameInFlight) const;

  private:
    Info m_Info;

    std::variant<std::monostate, std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>> m_RingData;
};

template <>
struct IsResource<Attachment> : std::true_type {};

} // namespace Physbuzz
