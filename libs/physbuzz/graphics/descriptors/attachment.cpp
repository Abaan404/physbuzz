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

    Image::UsageFlags usage;
    switch (m_Info.usage) {
    case Usage::Color:
        usage = Image::UsageFlagBits::eInputAttachment | Image::UsageFlagBits::eColorAttachment;
        break;

    case Usage::Depth:
    case Usage::Stencil:
    case Usage::DepthStencil:
        usage = Image::UsageFlagBits::eInputAttachment | Image::UsageFlagBits::eDepthStencilAttachment;
        break;
    }

    if (!m_Info.sampler.build()) {
        Logger::ERROR("[Texture] Failed to build a sampler.");
        return false;
    }

    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        Image image = {{}};

        switch (m_Info.type) {
        case Type::Dim2D:
            image = {{
                .usage = usage | Image::UsageFlagBits::eSampled | Image::UsageFlagBits::eTransferSrc | Image::UsageFlagBits::eTransferDst,
                .type = Image::Type::e2D,
                .format = m_Info.format,
            }};
            break;

        case Type::Cube:
            image = {{
                .usage = usage | Image::UsageFlagBits::eSampled | Image::UsageFlagBits::eTransferSrc | Image::UsageFlagBits::eTransferDst,
                .type = Image::Type::e2D,
                .arrayLayers = 6,
                .flags = Image::FlagBits::eCubeCompatible,
                .format = m_Info.format,
            }};
            break;
        }

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

    if (!m_Info.sampler.destroy()) {
        Logger::WARNING("[Texture] Failed to destroy sampler.");
        return false;
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
    vk::ImageAspectFlags aspect;

    switch (m_Info.usage) {
    case Usage::Color:
        aspect = vk::ImageAspectFlagBits::eColor;
        break;

    case Usage::Depth:
        aspect = vk::ImageAspectFlagBits::eDepth;
        break;

    case Usage::Stencil:
        aspect = vk::ImageAspectFlagBits::eStencil;
        break;

    case Usage::DepthStencil:
        aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        break;
    }

    vk::ImageSubresourceRange subresourceRange = {};
    vk::ImageViewType type = {};

    switch (m_Info.type) {
    case Type::Dim2D:
        type = vk::ImageViewType::e2D;
        subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        break;

    case Type::Cube:
        type = vk::ImageViewType::eCube;
        subresourceRange = {
            .aspectMask = aspect,
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
