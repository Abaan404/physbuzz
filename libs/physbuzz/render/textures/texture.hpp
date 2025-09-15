#pragma once

#include "../../io/image.hpp"
#include "../../resources/defines.hpp"
#include "../buffer.hpp"

namespace Physbuzz {

class Texture {
  public:
    struct Info {
        std::shared_ptr<Transfer> transfer;
        ImageFile::Info file;
    };

    Texture(const Info &info);

    bool build();
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
