#include "transfer.hpp"

#include "../app/application.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

TransferBatch::TransferBatch(const Info &info)
    : m_Info(info) {}

bool TransferBatch::add(const Buffer &buffer, std::vector<std::byte> &&bytes, std::uint64_t offset) {
    if (buffer.getData().bufferInfo.size > bytes.size() + offset) {
        Logger::ERROR(
            "[TransferBatch] Failed to map buffer using bytes ({}) and offset ({}) of size ({})",
            bytes.size(),
            offset,
            buffer.getData().bufferInfo.size);

        return false;
    }

    m_Info.buffers.emplace_back<BufferWrite>({
        .buffer = buffer,
        .bytes = bytes,
        .offset = offset,
    });

    return true;
}

bool TransferBatch::add(const Image &image, std::vector<std::byte> &&bytes) {
    // TODO bounds checking

    m_Info.images.emplace_back<ImageWrite>({
        .image = image,
        .bytes = bytes,
    });

    return true;
}

bool TransferBatch::add(const Image &image, const ImageFile::Info &imageFile) {
    // TODO bounds checking

    if (image.getInfo().arrayLayers != imageFile.files.size()) {
        Logger::ERROR("[TransferBatch] Image needs {} array layers to be written.", image.getInfo().arrayLayers);
        return false;
    }

    m_Info.imageFiles.emplace_back<ImageFileWrite>({
        .image = image,
        .imageFile = imageFile,
    });

    return true;
}

const TransferBatch::Info &TransferBatch::getInfo() const {
    return m_Info;
}

Transfer::Transfer()
    : m_Deletion({}) {}

bool Transfer::build() {
    m_Command.pool = PBZ_VK_CHECK(App::Device.createCommandPool({
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = App::Indices.transfer,
    }));

    m_Fences.submit = PBZ_VK_CHECK(App::Device.createFence({
        .flags = vk::FenceCreateFlagBits::eSignaled,
    }));

    m_Fences.immediate = PBZ_VK_CHECK(App::Device.createFence({
        .flags = vk::FenceCreateFlagBits::eSignaled,
    }));

    return true;
}

bool Transfer::destroy() {
    App::Device.destroyFence(m_Fences.submit);
    App::Device.destroyFence(m_Fences.immediate);

    App::Device.destroyCommandPool(m_Command.pool);
    m_Command.pool = nullptr;

    return true;
}

void Transfer::submit(const TransferBatch &batch) {
    ZoneScopedN("Transfer/Submit");

    vk::CommandBuffer submit = nullptr;

    // prepare a one time command buffer
    vk::CommandBufferAllocateInfo allocateInfo = {
        .commandPool = m_Command.pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    PBZ_VK_CHECK_RESULT(App::Device.allocateCommandBuffers(&allocateInfo, &submit));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));

    PBZ_VK_CHECK_RESULT(submit.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    for (const auto &write : batch.getInfo().buffers) {
        {
            vk::BufferMemoryBarrier2 barrier = {
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
                .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .buffer = write.buffer.getData().buffer,
                .offset = write.offset,
                .size = write.bytes.size(),
            };

            submit.pipelineBarrier2({
                .dependencyFlags = {},
                .bufferMemoryBarrierCount = 1,
                .pBufferMemoryBarriers = &barrier,
            });
        }

        if (!write.buffer.map(submit, &m_Deletion, write.bytes, write.offset)) {
            Logger::ERROR(
                "[Transfer] Failed to map buffer using bytes ({}) and offset ({}) of size ({})",
                write.bytes.size(),
                write.offset,
                write.buffer.getData().bufferInfo.size);
        }
    }

    for (const auto &write : batch.getInfo().images) {
        const Image::Data &imageData = write.image.getData();
        const Image::Info &imageInfo = write.image.getInfo();

        {
            vk::ImageMemoryBarrier2 barrier = {
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
                .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .image = imageData.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = imageInfo.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = imageInfo.arrayLayers,
                },
            };

            submit.pipelineBarrier2({
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier,
            });
        }

        if (!write.image.map(submit, &m_Deletion, write.bytes)) {
            Logger::ERROR("[Transfer] Failed to map image.");
        }

        {
            vk::ImageMemoryBarrier2 barrier = {
                .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eNone,
                .dstAccessMask = vk::AccessFlagBits2::eNone,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .image = imageData.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = imageInfo.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = imageInfo.arrayLayers,
                },
            };

            submit.pipelineBarrier2({
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier,
            });
        }
    }

    for (const auto &write : batch.getInfo().imageFiles) {
        ImageFile imageFile = write.imageFile;
        imageFile.read();

        const ImageFile::Data &imageFileData = imageFile.getData();

        const Image::Data &imageData = write.image.getData();
        const Image::Info &imageInfo = write.image.getInfo();

        if (imageData.imageInfo.extent.width != imageFileData.meta.resolution.x || imageData.imageInfo.extent.height != imageFileData.meta.resolution.y || imageData.imageInfo.extent.depth != 1) {
            Logger::ERROR("[Transfer] invalid image extent.");
            continue;
        }

        {
            vk::ImageMemoryBarrier2 barrier = {
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
                .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .image = imageData.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = imageInfo.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = imageInfo.arrayLayers,
                },
            };

            submit.pipelineBarrier2({
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier,
            });
        }

        if (!write.image.map(submit, &m_Deletion, imageFileData.image)) {
            Logger::ERROR("[Transfer] Failed to map image.");
        }

        {
            vk::ImageMemoryBarrier2 barrier = {
                .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eNone,
                .dstAccessMask = vk::AccessFlagBits2::eNone,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .image = imageData.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = imageInfo.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = imageInfo.arrayLayers,
                },
            };

            submit.pipelineBarrier2({
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier,
            });
        }
    }

    PBZ_VK_CHECK_RESULT(submit.end());

    // submit
    const vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &submit,
    };

    PBZ_VK_CHECK_RESULT(App::Queues.transfer.submit(submitInfo, m_Fences.submit));
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));

    // release the staging buffers
    m_Deletion.flush();
}

void Transfer::immediate(std::function<void(vk::CommandBuffer)> record) {
    // prepare the command buffer
    vk::CommandBuffer immediate = nullptr;

    // create immediate buffers
    vk::CommandBufferAllocateInfo allocateInfo = {
        .commandPool = m_Command.pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    PBZ_VK_CHECK_RESULT(App::Device.allocateCommandBuffers(&allocateInfo, &immediate));
    PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.immediate));

    PBZ_VK_CHECK_RESULT(immediate.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    }));

    record(immediate);

    PBZ_VK_CHECK_RESULT(immediate.end());

    vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &immediate,
    };

    PBZ_VK_CHECK_RESULT(App::Queues.graphics.submit(submitInfo, m_Fences.immediate));
    PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.immediate, vk::True, std::numeric_limits<std::uint64_t>::max()));

    App::Device.freeCommandBuffers(m_Command.pool, 1, &immediate);
    immediate = nullptr;
}

} // namespace Physbuzz
