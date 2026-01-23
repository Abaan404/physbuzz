#include "texture.hpp"

#include "../../app/application.hpp"
#include "../defines.hpp"
#include <span>

namespace Physbuzz {

Texture::Texture(const Info &info, std::optional<Image> image)
    : m_Info(info),
      m_Image(image.value_or(Image::Info{})) {}

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

bool Texture::build(const glm::uvec3 &resolution) {
    if (m_Data.view != nullptr) {
        Logger::WARNING("[Texture] Trying to construct a built texture.");
        return true;
    }

    switch (m_Info.type) {
    case Type::Dim2D:
        m_Image = {{
            .usage = Image::ImageUsageFlagBits::eSampled | Image::ImageUsageFlagBits::eTransferSrc | Image::ImageUsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .format = m_Info.format,
        }};
        break;

    case Type::Attachment:
        m_Image = {{
            .usage = Image::ImageUsageFlagBits::eSampled | Image::ImageUsageFlagBits::eColorAttachment | Image::ImageUsageFlagBits::eTransferSrc | Image::ImageUsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .format = m_Info.format,
        }};

        break;
    }

    if (!m_Image.build(resolution)) {
        Logger::ERROR("[Texture] Failed to build image.");
        return false;
    }

    m_Data = {
        .view = createImageView(),
        .sampler = createSampler(),
    };

    return true;
}

bool Texture::destroy() {
    if (!m_Data.view) {
        Logger::WARNING("[Texture] Trying to destroy a destructed texture.");
        return true;
    }

    App::Device.destroyImageView(m_Data.view);
    m_Data.view = nullptr;

    // Note: samplers are destroyed on engine shutdown, could refcount it but unnecessary for this scope
    // In the future samplers could be its own resource and it could be specially handled as such
    m_Data.sampler = nullptr;

    return m_Image.destroy();
}

bool Texture::resize(const RenderContext &context, const glm::uvec3 &size) {
    Image image = m_Image.getInfo();

    // create new image
    if (!image.build(size)) {
        Logger::ERROR("[Texture] Failed to resize new texture.");
        return false;
    }

    // copy old data to the new image
    std::vector<vk::ImageCopy> copies = {{
        .srcSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = image.getInfo().arrayLayers,
        },
        .srcOffset = {},
        .dstSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = image.getInfo().arrayLayers,
        },
        .dstOffset = {},
        .extent = {
            .width = glm::min(m_Image.getData().imageInfo.extent.width, image.getData().imageInfo.extent.width),
            .height = glm::min(m_Image.getData().imageInfo.extent.height, image.getData().imageInfo.extent.height),
            .depth = glm::min(m_Image.getData().imageInfo.extent.depth, image.getData().imageInfo.extent.depth),
        },
    }};

    image.copy(context.command, m_Image, copies);

    // mark old image for deferred deletion and update
    context.deletionQueue->enqueue(std::move(m_Image));
    context.deletionQueue->enqueue(m_Data.view);
    // sampler is erased on app exit
    // context.deletionQueue->enqueue(m_Data.sampler);

    m_Image = image;

    m_Data = {
        .view = createImageView(),
        .sampler = createSampler(),
    };

    return true;
}

const Texture::Info &Texture::getInfo() const {
    return m_Info;
}

const Texture::Data &Texture::getData() const {
    return m_Data;
}

const Image &Texture::getImage() const {
    return m_Image;
}

glm::uvec3 Texture::getSize() const {
    return glm::uvec3(m_Image.getData().imageInfo.extent.width, m_Image.getData().imageInfo.extent.height, m_Image.getData().imageInfo.extent.depth);
}

vk::Sampler Texture::createSampler() const {
    switch (m_Info.sampler) {
    case Sampler::Linear:
        static vk::Sampler linear = PBZ_VK_CHECK(App::Device.createSampler({
            .flags = {},
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .mipLodBias = 0.0f,
            .anisotropyEnable = vk::True,
            .maxAnisotropy = App::PhysicalDeviceProperties.limits.maxSamplerAnisotropy,
            .compareEnable = vk::False,
            .compareOp = vk::CompareOp::eAlways,
            .minLod = 0.0f,
            .maxLod = 1.0f,
            .borderColor = vk::BorderColor::eIntOpaqueBlack,
            .unnormalizedCoordinates = vk::False,
        }));

        static std::once_flag flag;
        std::call_once(flag, []() {
            App::Deletion.enqueue(linear);
        });

        return linear;

    case Sampler::None:
        return nullptr;
    }
}

vk::ImageView Texture::createImageView() const {
    switch (m_Info.type) {
    case Type::Dim2D:
    case Type::Attachment:
        return PBZ_VK_CHECK(App::Device.createImageView({
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
    }

    return nullptr;
}

} // namespace Physbuzz
