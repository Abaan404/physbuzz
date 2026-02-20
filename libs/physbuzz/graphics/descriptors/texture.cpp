#include "texture.hpp"

#include "../../app/application.hpp"
#include "../../events/descriptor.hpp"
#include "../transfer.hpp"

namespace Physbuzz {

Texture::Texture(const Info &info)
    : m_Info(info) {}

bool Texture::build(std::vector<ImageFile::Info> imageInfos, const std::shared_ptr<Transfer> transfer) {
    if (imageInfos.empty()) {
        Logger::ERROR("[Texture] No images provided.");
        return false;
    }

    if (transfer == nullptr) {
        Logger::ERROR("[Texture] No transfer system provided for texture.");
        return false;
    }

    std::vector<ImageFile> imageFiles;
    imageFiles.reserve(imageInfos.size());

    // read every image
    for (const auto &imageInfo : imageInfos) {
        ImageFile &imageFile = imageFiles.emplace_back(imageInfo);
        if (!imageFile.read()) {
            Logger::ERROR("[Texture] Could not read image file: '{}'", imageInfo.file.path.string());
            return false;
        }
    }

    glm::uvec2 resolution = imageFiles.begin()->getData().resolution;
    std::size_t bufferSize = 0;

    // validate resolution
    for (const auto &imageFile : imageFiles) {
        const ImageFile::Data &imageData = imageFile.getData();

        if (imageData.resolution != resolution) {
            Logger::ERROR("[Texture] Uneven texture resolution in images.");
            return false;
        }

        bufferSize += imageFile.getData().image.size();
    }

    std::vector<std::byte> bytes;
    bytes.reserve(bufferSize);

    // measure sizes
    for (const auto &imageFile : imageFiles) {
        const ImageFile::Data &imageData = imageFile.getData();

        bytes.insert(bytes.end(), std::make_move_iterator(imageData.image.begin()), std::make_move_iterator(imageData.image.end()));
    }

    build({resolution, 1.0f});

    // can only inspect this after building an image type
    if (m_Data.image.getInfo().arrayLayers != imageFiles.size()) {
        destroy();
        Logger::ERROR("[Texture] Incorrect image layers provided (required {} got {}).", m_Data.image.getInfo().arrayLayers, imageFiles.size());
        return false;
    }

    transfer->map(m_Data.image, bytes, vk::ImageLayout::eShaderReadOnlyOptimal);

    return true;
}

bool Texture::build(const glm::uvec3 &resolution) {
    if (m_Data.view != nullptr) {
        Logger::WARNING("[Texture] Trying to construct a built texture.");
        return true;
    }

    Image image = {{}};
    Sampler sampler = m_Info.sampler;

    if (!sampler.build()) {
        Logger::ERROR("[Texture] Failed to build a sampler.");
        return false;
    }

    switch (m_Info.type) {
    case Type::Dim2D:
        image = {{
            .usage = Image::UsageFlagBits::eSampled | Image::UsageFlagBits::eTransferSrc | Image::UsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .format = m_Info.format,
        }};
        break;

    case Type::Cube:
        image = {{
            .usage = Image::UsageFlagBits::eSampled | Image::UsageFlagBits::eTransferSrc | Image::UsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .arrayLayers = 6,
            .flags = Image::FlagBits::eCubeCompatible,
            .format = m_Info.format,
        }};
        break;
    }

    if (!image.build(resolution)) {
        Logger::ERROR("[Texture] Failed to build image.");
        return false;
    }

    const auto &[view, subresourceRange] = createImageView(image);

    m_Data = {
        .sampler = sampler,
        .image = image,
        .view = view,
        .subresourceRange = subresourceRange,
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

    if (!m_Data.sampler.destroy()) {
        Logger::WARNING("[Texture] Failed to destroy sampler.");
        return false;
    }

    if (!m_Data.image.destroy()) {
        Logger::WARNING("[Texture] Failed to destroy image.");
        return false;
    }

    return true;
}

bool Texture::rebuild(const RenderContext &context, const glm::uvec3 &size) {
    Image image = m_Data.image.getInfo();

    // create new image
    if (!image.build(size)) {
        Logger::ERROR("[Texture] Failed to resize new texture.");
        return false;
    }

    // mark old image for deferred deletion and update
    context.deletionQueue->enqueue(std::move(m_Data.image));
    context.deletionQueue->enqueue(m_Data.view);
    // sampler is erased on app exit
    // context.deletionQueue->enqueue(m_Data.sampler.getData().sampler);

    const auto &[view, subresourceRange] = createImageView(image);

    m_Data = {
        .sampler = m_Data.sampler,
        .image = image,
        .view = view,
        .subresourceRange = subresourceRange,
    };

    notifyCallbacks<OnTextureRebuild>({
        .texture = this,
    });

    return true;
}

const Texture::Info &Texture::getInfo() const {
    return m_Info;
}

const Texture::Data &Texture::getData() const {
    return m_Data;
}

glm::uvec3 Texture::getSize() const {
    return glm::uvec3(m_Data.image.getData().imageInfo.extent.width, m_Data.image.getData().imageInfo.extent.height, m_Data.image.getData().imageInfo.extent.depth);
}

std::tuple<vk::ImageView, vk::ImageSubresourceRange> Texture::createImageView(const Image &image) const {
    vk::ImageSubresourceRange subresourceRange = {};
    vk::ImageViewType type = {};

    switch (m_Info.type) {
    case Type::Dim2D:
        type = vk::ImageViewType::e2D;
        subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        break;

    case Type::Cube:
        type = vk::ImageViewType::eCube;
        subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 6,
        };
        break;
    }

    vk::ImageView view = PBZ_VK_CHECK(App::Device.createImageView({
        .flags = {},
        .image = image.getData().image,
        .viewType = type,
        .format = image.getInfo().format,
        .components = {},
        .subresourceRange = subresourceRange,
    }));

    return std::make_tuple(view, subresourceRange);
}

} // namespace Physbuzz
