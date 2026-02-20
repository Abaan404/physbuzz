#include "attachment.hpp"

#include "../../app/application.hpp"
#include "../../app/deletion.hpp"
#include "../../debug/macros.hpp"
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

        const auto &[view, subresourceRange] = createImageView(image);
        Data &data = ringData.emplace_back<Data>({
            .image = image,
            .view = view,
            .subresourceRange = subresourceRange,
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

bool Attachment::rebuild(const RenderContext &context, const glm::uvec2 &size) {
    Data &data = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData)[context.frameInFlight];

    Image image = data.image.getInfo();

    // create new image
    if (!image.build({size, 1})) {
        Logger::ERROR("[Attachment] Failed to resize new attachment.");
        return false;
    }

    // mark old image for deferred deletion and update
    context.deletionQueue->enqueue(std::move(data.image));
    context.deletionQueue->enqueue(data.view);

    const auto &[view, subresourceRange] = createImageView(image);

    data = {
        .image = image,
        .view = view,
        .subresourceRange = subresourceRange,
    };

    notifyCallbacks<OnAttachmentRebuild>({
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
    PBZ_ASSERT(frameInFlight < detail::MAX_FRAMES_IN_FLIGHT, "[DynamicBuffer] Invalid frame in flight");
    const Data &data = getRingData()[frameInFlight];

    return {
        data.image.getData().imageInfo.extent.width,
        data.image.getData().imageInfo.extent.height,
    };
}

std::tuple<vk::ImageView, vk::ImageSubresourceRange> Attachment::createImageView(const Image &image) const {
    vk::ImageSubresourceRange subresourceRange = {};
    vk::ImageViewType type = {};

    switch (m_Info.type) {
    case Type::Color:
        type = vk::ImageViewType::e2D;
        subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        break;

    case Type::Depth:
        type = vk::ImageViewType::e2D;
        subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        break;

    case Type::Stencil:
        type = vk::ImageViewType::e2D;
        subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eStencil,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        break;

    case Type::DepthStencil:
        type = vk::ImageViewType::e2D;
        subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
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
