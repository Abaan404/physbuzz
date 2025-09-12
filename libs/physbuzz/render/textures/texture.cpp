#include "texture.hpp"

namespace Physbuzz {

Texture::Texture(const Info &info)
    : m_Info(info),
      m_Image(info.image) {}

bool Texture::build() {
    if (m_Info.file.file.path.empty()) {
        return false;
    }

    // // OpenGL's origin for textures are on its top-left
    // m_Info.image.flipVertically = true;
    ImageFile image = ImageFile(m_Info.file);
    if (!image.build()) {
        Logger::ERROR("[Texture] Could not build image: {}", m_Info.file.file.path.string());
        return false;
    }

    if (!image.read()) {
        Logger::ERROR("[Texture] Could not load image: {}", m_Info.file.file.path.string());
        image.destroy();
        return false;
    }

    const ImageFile::Data imageData = image.getData();

    m_Image.build({imageData.resolution, 1});

    if (!image.destroy()) {
        Logger::ERROR("[Texture] Could not destroy image: {}", m_Info.file.file.path.string());
        return false;
    }

    m_View = PBZ_VK_CHECK(App::Device.createImageView({
        .flags = {},
        .image = m_Image.getData().image,
        .viewType = vk::ImageViewType::e2D,
        .format = m_Image.getInfo().format,
        .components = {},
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    }));

    vk::PhysicalDeviceProperties properties = App::PhysicalDevice.getProperties();

    m_Sampler = PBZ_VK_CHECK(App::Device.createSampler({
        .flags = {},
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0,
        .anisotropyEnable = 1,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False,
    }));

    return true;
}

bool Texture::destroy() {
    App::Device.destroySampler(m_Sampler);
    m_Sampler = nullptr;

    App::Device.destroyImageView(m_View);
    m_View = nullptr;

    return m_Image.destroy();
}

const Texture::Info &Texture::getInfo() const {
    return m_Info;
}

const Image &Texture::getImage() const {
    return m_Image;
}

const vk::ImageView &Texture::getImageView() const {
    return m_View;
}

const vk::Sampler &Texture::getSampler() const {
    return m_Sampler;
}

} // namespace Physbuzz
