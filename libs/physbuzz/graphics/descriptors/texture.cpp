#include "texture.hpp"

#include "../../app/application.hpp"
#include "../../events/descriptor.hpp"
#include "../transfer.hpp"

namespace Physbuzz {

Texture::Texture(const Info &info)
    : m_Info(info) {}

bool Texture::build(const glm::uvec3 &resolution) {
    if (m_Data.image.getData().image != nullptr) {
        Logger::WARNING("[Texture] Trying to construct a built texture.");
        return true;
    }

    Image image = {{}};
    if (!m_Info.sampler.build()) {
        Logger::ERROR("[Texture] Failed to build a sampler.");
        return false;
    }

    std::uint32_t mipLevels = std::floor(std::log2(std::max(resolution.x, resolution.y))) + 1;
    vk::ImageUsageFlags usage = Image::UsageFlags::eTransferSrc | Image::UsageFlags::eTransferDst;

    switch (m_Info.usage) {
    case Usage::Sampled:
        usage |= Image::UsageFlags::eSampled;
        break;

    case Usage::Storage:
        // most likely this texture would be sampled anyways
        usage |= Image::UsageFlags::eSampled | Image::UsageFlags::eStorage;
        mipLevels = 1;
        break;
    }

    std::vector<Image::ViewInfo> views = m_Info.additionalViews;

    switch (m_Info.type) {
    case Type::Dim2D:
        views.emplace_back<Image::ViewInfo>({
            .type = Physbuzz::Image::ViewType::e2D,
            .subresourceRange = {
                .aspectMask = Physbuzz::Image::AspectFlags::eColor,
                .levelCount = Physbuzz::Image::RemainingMipLevels,
                .layerCount = 1,
            },
        });

        image = {{
            .usage = usage,
            .type = Image::Type::e2D,
            .mipLevels = mipLevels,
            .format = m_Info.format,
            .views = views,
        }};
        break;

    case Type::Cube:
        views.emplace_back<Image::ViewInfo>({
            .type = Physbuzz::Image::ViewType::eCube,
            .subresourceRange = {
                .aspectMask = Physbuzz::Image::AspectFlags::eColor,
                .levelCount = Physbuzz::Image::RemainingMipLevels,
                .layerCount = 6,
            },
        });

        image = {{
            .usage = usage,
            .type = Image::Type::e2D,
            .mipLevels = mipLevels,
            .arrayLayers = 6,
            .flags = Image::FlagBits::eCubeCompatible,
            .format = m_Info.format,
            .views = views,
        }};
        break;
    }

    if (!image.build(resolution)) {
        Logger::ERROR("[Texture] Failed to build image.");
        return false;
    }

    m_Data = {
        .image = image,
    };

    return true;
}

bool Texture::destroy() {
    if (m_Data.image.getData().image == nullptr) {
        Logger::WARNING("[Texture] Trying to destroy a destructed texture.");
        return true;
    }

    if (!m_Info.sampler.destroy()) {
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
    // sampler is erased on app exit
    // context.deletionQueue->enqueue(m_Data.sampler.getData().sampler);

    m_Data = {
        .image = image,
    };

    notifyCallbacks<OnTextureRebuild>({
        .texture = this,
        .context = context,
    });

    return true;
}

bool Texture::write(const ImageFile::Info &imageFile, TransferBatch &batch) {
    return batch.add(m_Data.image, imageFile);
}

bool Texture::write(std::vector<std::byte> &&bytes, TransferBatch &batch) {
    return batch.add(m_Data.image, std::move(bytes));
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

} // namespace Physbuzz
