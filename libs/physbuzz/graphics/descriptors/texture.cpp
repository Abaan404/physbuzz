#include "texture.hpp"

#include "../../app/application.hpp"
#include "../../events/descriptor.hpp"

namespace Physbuzz {

Texture::Texture(const Info &info)
    : m_Info(info) {}

bool Texture::build(std::vector<ImageFile::Info> imageInfos, std::shared_ptr<Transfer> transfer) {
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
    if (m_Image.getInfo().arrayLayers != imageFiles.size()) {
        destroy();
        Logger::ERROR("[Texture] Incorrect image layers provided (required {} got {}).", m_Image.getInfo().arrayLayers, imageFiles.size());
        return false;
    }

    transfer->map(m_Image, bytes, m_Data.layout);

    return true;
}

bool Texture::build(const glm::uvec3 &resolution) {
    if (m_Data.view != nullptr) {
        Logger::WARNING("[Texture] Trying to construct a built texture.");
        return true;
    }

    Sampler sampler = m_Info.sampler;

    if (!sampler.build()) {
        Logger::ERROR("[Texture] Failed to build a sampler.");
        return false;
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
            .usage = Image::ImageUsageFlagBits::eSampled | Image::ImageUsageFlagBits::eColorAttachment | Image::ImageUsageFlagBits::eInputAttachment | Image::ImageUsageFlagBits::eTransferSrc | Image::ImageUsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .format = m_Info.format,
        }};
        break;

    case Type::Cube:
        m_Image = {{
            .usage = Image::ImageUsageFlagBits::eSampled | Image::ImageUsageFlagBits::eTransferSrc | Image::ImageUsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .arrayLayers = 6,
            .flags = Image::FlagBits::eCubeCompatible,
            .format = m_Info.format,
        }};
        break;
    }

    if (!m_Image.build(resolution)) {
        Logger::ERROR("[Texture] Failed to build image.");
        return false;
    }

    m_Data = {
        .sampler = sampler,
        .view = createImageView(),
        .layout = createLayout(),
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

    if (!m_Image.destroy()) {
        Logger::WARNING("[Texture] Failed to destroy image.");
        return false;
    }

    return true;
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

    image.copy(context.command, m_Image, copies, m_Data.layout);

    // mark old image for deferred deletion and update
    context.deletionQueue->enqueue(std::move(m_Image));
    context.deletionQueue->enqueue(m_Data.view);
    // sampler is erased on app exit
    // context.deletionQueue->enqueue(m_Data.sampler);

    m_Image = image;

    m_Data = {
        .sampler = m_Data.sampler,
        .view = createImageView(),
        .layout = createLayout(),
    };

    notifyCallbacks<OnTextureRealloc>({
        .texture = this,
        .context = context,
    });

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
    case Type::Cube:
        return PBZ_VK_CHECK(App::Device.createImageView({
            .flags = {},
            .image = m_Image.getData().image,
            .viewType = vk::ImageViewType::eCube,
            .format = m_Image.getInfo().format,
            .components = {},
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 6,
            },
        }));
    }

    return nullptr;
}

vk::ImageLayout Texture::createLayout() const {
    switch (m_Info.type) {
    case Type::Dim2D:
        return vk::ImageLayout::eShaderReadOnlyOptimal;
    case Type::Attachment:
        return vk::ImageLayout::eAttachmentOptimal;
    case Type::Cube:
        return vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    return vk::ImageLayout::eUndefined;
}

} // namespace Physbuzz
