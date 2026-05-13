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

    Image::Usage usage;
    switch (m_Info.usage) {
    case Usage::Color:
        usage = Image::UsageFlags::eInputAttachment | Image::UsageFlags::eColorAttachment;
        break;

    case Usage::Depth:
    case Usage::Stencil:
    case Usage::DepthStencil:
        usage = Image::UsageFlags::eInputAttachment | Image::UsageFlags::eDepthStencilAttachment;
        break;
    }

    if (!m_Info.sampler.build()) {
        Logger::ERROR("[Attachment] Failed to build a sampler.");
        return false;
    }

    for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        Image image = {{}};

        switch (m_Info.type) {
        case Type::Dim2D:
            image = {{
                .usage = usage | Image::UsageFlags::eSampled | Image::UsageFlags::eTransferSrc | Image::UsageFlags::eTransferDst,
                .type = Image::Type::e2D,
                .format = m_Info.format,
                .views = m_Info.views,
            }};
            break;

        case Type::Cube:
            image = {{
                .usage = usage | Image::UsageFlags::eSampled | Image::UsageFlags::eTransferSrc | Image::UsageFlags::eTransferDst,
                .type = Image::Type::e2D,
                .arrayLayers = 6,
                .flags = Image::FlagBits::eCubeCompatible,
                .format = m_Info.format,
                .views = m_Info.views,
            }};
            break;
        }

        if (!image.build({resolution, 1})) {
            for (auto &data : ringData) {
                data.image.destroy();
            }

            return false;
        }

        Data &data = ringData.emplace_back<Data>({
            .image = image,
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
        Logger::WARNING("[Attachment] Failed to destroy sampler.");
        return false;
    }

    std::array<Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);

    bool success = true;
    for (auto &data : ringData) {
        success &= data.image.destroy();
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

    data = {
        .image = image,
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
    PBZ_ASSERT(!std::holds_alternative<std::monostate>(m_RingData), "[Attachment] Image has not been allocated.");
    return std::get<std::array<Data, detail::MAX_FRAMES_IN_FLIGHT>>(m_RingData);
}

glm::uvec2 Attachment::getSize(std::uint32_t frameInFlight) const {
    PBZ_ASSERT(frameInFlight < detail::MAX_FRAMES_IN_FLIGHT, "[Attachment] Invalid frame in flight");
    const Data &data = getRingData()[frameInFlight];

    return {
        data.image.getData().imageInfo.extent.width,
        data.image.getData().imageInfo.extent.height,
    };
}

} // namespace Physbuzz
