#pragma once

#include "../../io/image.hpp"
#include "../../resources/defines.hpp"
#include "../buffer.hpp"

namespace Physbuzz {

class Texture {
  public:
    struct Info {
        Image::Info image;
    };

    static constexpr Info Tex2D = {
        .image = {
            .usage = Image::ImageUsageFlagBits::eSampled | Image::ImageUsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .mipLevels = 1,
            .arrayLayers = 1,
        },
    };

    static constexpr Info TexCubemap = {
        .image = {
            .usage = Image::ImageUsageFlagBits::eSampled | Image::ImageUsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .mipLevels = 1,
            .arrayLayers = 6,
            .flags = Image::FlagBits::eCubeCompatible,
        },
    };

    Texture(const Info &info);

    bool build(ImageFile::Info imageInfo, std::shared_ptr<Transfer> transfer);
    bool build(const glm::uvec3 &resolution);
    bool destroy();

    const Info &getInfo() const;
    const Image &getImage() const;

    const vk::ImageView &getImageView() const;
    const vk::Sampler &getSampler() const;

  private:
    Info m_Info;
    Image m_Image;

    vk::ImageView m_View;
    vk::Sampler m_Sampler;
};

template <>
struct IsResource<Texture> : std::true_type {};

} // namespace Physbuzz
