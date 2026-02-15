#pragma once

#include "../../events/handler.hpp"
#include "../../io/image.hpp"
#include "../../resources/defines.hpp"
#include "../defines.hpp"
#include "../memory.hpp"
#include "sampler.hpp"

namespace Physbuzz {

class Texture : public EventSubject {
  public:
    using Format = vk::Format;

    enum class Type {
        Attachment,
        Cube,
        Dim2D,
    };

    struct Info {
        Type type;
        Sampler::Info sampler;
        Format format = Format::eR8G8B8A8Unorm;
    };

    struct Data {
        Sampler sampler = {{Sampler::Type::None}};
        Image image = {{}};

        vk::ImageView view = nullptr;
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
    };

    Texture(const Info &info);

    bool build(std::vector<ImageFile::Info> imageInfo, std::shared_ptr<Transfer> transfer);
    bool build(const glm::uvec3 &resolution);
    bool destroy();

    bool resize(const RenderContext &context, const glm::uvec3 &size);

    const Info &getInfo() const;
    const Data &getData() const;

    glm::uvec3 getSize() const;

  private:
    vk::ImageView createImageView(const Image &image) const;
    vk::ImageLayout createLayout() const;

    Info m_Info;
    Data m_Data;
};

template <>
struct IsResource<Texture> : std::true_type {};

} // namespace Physbuzz
