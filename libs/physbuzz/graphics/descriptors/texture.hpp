#pragma once

#include "../../io/image.hpp"
#include "../../resources/defines.hpp"
#include "../memory.hpp"

namespace Physbuzz {

struct RenderContext;

class Texture {
  public:
    using Format = vk::Format;

    enum class Type {
        Attachment,
        Dim2D,
    };

    enum class Sampler {
        Linear,
        None,
    };

    struct Info {
        Type type;
        Sampler sampler = Sampler::Linear;
        Format format = Format::eR8G8B8A8Unorm;
    };

    struct Data {
        vk::ImageView view = nullptr;
        vk::Sampler sampler = nullptr;
    };

    Texture(const Info &info, std::optional<Image> image = std::nullopt);

    bool build(ImageFile::Info imageInfo, std::shared_ptr<Transfer> transfer);
    bool build(const glm::uvec3 &resolution);
    bool destroy();

    bool resize(const RenderContext &context, const glm::uvec3 &size);

    const Info &getInfo() const;
    const Data &getData() const;
    const Image &getImage() const;

    glm::uvec3 getSize() const;

  private:
    vk::Sampler createSampler() const;
    vk::ImageView createImageView() const;

    Info m_Info;

    Image m_Image;
    Data m_Data;
};

template <>
struct IsResource<Texture> : std::true_type {};

} // namespace Physbuzz
