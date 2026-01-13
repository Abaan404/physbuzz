#include "texture.hpp"

#include "../../app/application.hpp"
#include <span>

namespace Physbuzz {

Texture::Texture(const Info &info)
    : m_Info(info),
      m_Image(info.image) {}

bool Texture::build(ImageFile::Info imageInfo, std::shared_ptr<Transfer> transfer) {
    if (imageInfo.file.path.empty()) {
        return false;
    }

    if (transfer == nullptr) {
        Logger::ERROR("[Texture] No transfer system provided for texture.");
        return false;
    }

    ImageFile imageFile = ImageFile(imageInfo);
    if (!imageFile.build()) {
        Logger::ERROR("[Texture] Could not build image: {}", imageInfo.file.path.string());
        return false;
    }

    if (!imageFile.read()) {
        Logger::ERROR("[Texture] Could not load image: {}", imageInfo.file.path.string());
        imageFile.destroy();
        return false;
    }

    const ImageFile::Data imageData = imageFile.getData();

    build({imageData.resolution, 1});
    transfer->map(m_Image, imageData.image);

    if (!imageFile.destroy()) {
        Logger::ERROR("[Texture] Could not destroy image: {}", imageInfo.file.path.string());
        return false;
    }

    return true;
}

bool Texture::build(const glm::uvec3 &extent) {
    if (m_View) {
        Logger::WARNING("[Texture] Trying to construct a built texture.");
        return true;
    }

    if (!m_Image.build(extent)) {
        Logger::ERROR("[Texture] Failed to build image.");
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
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = static_cast<float>(m_Info.image.mipLevels),
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False,
    }));

    return true;
}

bool Texture::destroy() {
    if (!m_View) {
        Logger::WARNING("[Texture] Trying to destroy a destructed texture.");
        return true;
    }

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
