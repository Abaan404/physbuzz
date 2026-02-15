#include "attachment.hpp"

#include "../../app/application.hpp"
#include "../../events/descriptor.hpp"

namespace Physbuzz {

Attachment::Attachment(const Info &info)
    : m_Info(info), m_RingData(std::monostate()) {}

bool Attachment::build(const glm::uvec2 &resolution) {
    if (!std::holds_alternative<std::monostate>(m_RingData)) {
        Logger::WARNING("[Attachment] Trying to build a constructed attachment.");
        return true;
    }

    std::vector<Data> ringData;
    ringData.reserve(detail::MAX_FRAMES_IN_FLIGHT);

    Image::UsageFlagBits usage;
    switch (m_Info.type) {
    case Type::Color:
        usage = Image::UsageFlagBits::eColorAttachment;
        break;

    case Type::Depth:
    case Type::Stencil:
    case Type::DepthStencil:
        usage = Image::UsageFlagBits::eDepthStencilAttachment;
        break;
    }

    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        Image image = {{
            .usage = usage | Image::UsageFlagBits::eSampled | Image::UsageFlagBits::eInputAttachment | Image::UsageFlagBits::eTransferSrc | Image::UsageFlagBits::eTransferDst,
            .type = Image::Type::e2D,
            .format = m_Info.format,
        }};

        if (!image.build({resolution, 1})) {
            for (auto &data : ringData) {
                data.image.destroy();
                App::Device.destroyImageView(data.view);
            }

            return false;
        }

        Data &data = ringData.emplace_back<Data>({
            .image = image,
            .view = createImageView(image),
            .layout = createLayout(),
        });
    }

    // use pack expansion to create an array from this vector
    auto makeArray = []<std::size_t... I>(const std::vector<Data> &data, std::index_sequence<I...>) {
        return std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>{data[I]...};
    };

    m_RingData = makeArray(ringData, std::make_index_sequence<detail::MAX_FRAMES_IN_FLIGHT>{});

    return true;
}

bool Attachment::destroy() {
    if (std::holds_alternative<std::monostate>(m_RingData)) {
        Logger::WARNING("[Attachment] Trying to destroy a destructed attachment.");
        return true;
    }

    std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);

    bool success = true;
    for (auto &data : ringData) {
        success &= data.image.destroy();
        App::Device.destroyImageView(data.view);
    }

    if (success) {
        m_RingData = std::monostate();
    }

    return success;
}

bool Attachment::resize(const RenderContext &context, const glm::uvec2 &size) {
    Data &data = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData)[context.frameInFlight];

    Image image = data.image.getInfo();

    // create new image
    if (!image.build({size, 1})) {
        Logger::ERROR("[Attachment] Failed to resize new attachment.");
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
            .width = glm::min(data.image.getData().imageInfo.extent.width, image.getData().imageInfo.extent.width),
            .height = glm::min(data.image.getData().imageInfo.extent.height, image.getData().imageInfo.extent.height),
            .depth = glm::min(data.image.getData().imageInfo.extent.depth, image.getData().imageInfo.extent.depth),
        },
    }};

    image.copy(context.command, data.image, copies, data.layout);

    // mark old image for deferred deletion and update
    context.deletionQueue->enqueue(std::move(data.image));
    context.deletionQueue->enqueue(data.view);

    data = {
        .image = image,
        .view = createImageView(image),
        .layout = createLayout(),
    };

    notifyCallbacks<OnAttachmentRealloc>({
        .attachment = this,
        .context = context,
    });

    return true;
}

const Attachment::Info &Attachment::getInfo() const {
    return m_Info;
}

const std::array<Attachment::Data, detail::MAX_FRAMES_IN_FLIGHT> &Attachment::getRingData() const {
    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_RingData), "[DynamicBuffer] Buffer has not been allocated.");
    return std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);
}

glm::uvec2 Attachment::getSize(std::uint32_t frameInFlight) const {
    PBZ_ASSERT(frameInFlight < detail::MAX_FRAMES_IN_FLIGHT, "[ImGuiRenderer] Invalid frame in flight");
    const std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);

    return {
        ringData[frameInFlight].image.getData().imageInfo.extent.width,
        ringData[frameInFlight].image.getData().imageInfo.extent.height,
    };
}

vk::ImageView Attachment::createImageView(const Image &image) const {
    switch (m_Info.type) {
    case Type::Color:
        return PBZ_VK_CHECK(App::Device.createImageView({
            .flags = {},
            .image = image.getData().image,
            .viewType = vk::ImageViewType::e2D,
            .format = image.getInfo().format,
            .components = {},
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        }));

    case Type::Depth:
        return PBZ_VK_CHECK(App::Device.createImageView({
            .flags = {},
            .image = image.getData().image,
            .viewType = vk::ImageViewType::e2D,
            .format = image.getInfo().format,
            .components = {},
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eDepth,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        }));

    case Type::Stencil:
        return PBZ_VK_CHECK(App::Device.createImageView({
            .flags = {},
            .image = image.getData().image,
            .viewType = vk::ImageViewType::e2D,
            .format = image.getInfo().format,
            .components = {},
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eStencil,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        }));

    case Type::DepthStencil:
        return PBZ_VK_CHECK(App::Device.createImageView({
            .flags = {},
            .image = image.getData().image,
            .viewType = vk::ImageViewType::e2D,
            .format = image.getInfo().format,
            .components = {},
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        }));
    }

    return nullptr;
}

vk::ImageLayout Attachment::createLayout() const {
    return vk::ImageLayout::eAttachmentOptimal;
}

} // namespace Physbuzz
