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

Transfer::Transfer(const Info &info)
    : m_Info(info) {}

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

template <typename T>
void Transfer::submit(const std::vector<T> &writes, std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const T &)> record) {
    DeletionQueue transientQueue;

    std::size_t index = 0;
    while (index < writes.size()) {
        std::size_t chunkSize = 0;
        vk::CommandBuffer submit = nullptr;

        // prepare a buffer for this chunk
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

        while (index < writes.size() && chunkSize < m_Info.maxChunkSize) {
            chunkSize += record(submit, transientQueue, writes.at(index));
            index++;
        }

        PBZ_VK_CHECK_RESULT(submit.end());

        // submit
        const vk::SubmitInfo submitInfo = {
            .commandBufferCount = 1,
            .pCommandBuffers = &submit,
        };

        PBZ_VK_CHECK_RESULT(App::Queues.transfer.submit(submitInfo, m_Fences.submit));

        // wait for submission
        PBZ_VK_CHECK_RESULT(App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max()));
        PBZ_VK_CHECK_RESULT(App::Device.resetFences(m_Fences.submit));

        // free the buffer
        App::Device.freeCommandBuffers(m_Command.pool, 1, &submit);
        submit = nullptr;

        transientQueue.flush();
    }
}

void Transfer::submit(const TransferBatch &batch) {
    ZoneScopedN("Transfer/Submit");

    submit<TransferBatch::BufferWrite>(
        batch.getInfo().buffers,
        [](vk::CommandBuffer submit, DeletionQueue &transientQueue, const TransferBatch::BufferWrite &write) {
            vk::BufferMemoryBarrier2 barrier = {
                .srcStageMask = vk::PipelineStageFlagBits2::eHost,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
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

            if (!write.buffer.map(submit, &transientQueue, write.bytes, write.offset)) {
                Logger::ERROR(
                    "[Transfer] Failed to map buffer using bytes ({}) and offset ({}) of size ({})",
                    write.bytes.size(),
                    write.offset,
                    write.buffer.getData().bufferInfo.size);

                return 0ul;
            }

            return write.bytes.size();
        });

    submit<TransferBatch::ImageWrite>(
        batch.getInfo().images,
        [](vk::CommandBuffer submit, DeletionQueue &transientQueue, const TransferBatch::ImageWrite &write) {
            const Image::Data &imageData = write.image.getData();
            const Image::Info &imageInfo = write.image.getInfo();

            vk::ImageMemoryBarrier2 barrierPre = {
                .srcStageMask = vk::PipelineStageFlagBits2::eHost,
                .srcAccessMask = vk::AccessFlagBits2::eHostRead,
                .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
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
                .pImageMemoryBarriers = &barrierPre,
            });

            if (!write.image.map(submit, &transientQueue, write.bytes)) {
                Logger::ERROR("[Transfer] Failed to map image.");
                return 0ul;
            }

            vk::ImageMemoryBarrier2 barrierPost = {
                .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
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
                .pImageMemoryBarriers = &barrierPost,
            });

            return write.bytes.size();
        });

    submit<TransferBatch::ImageFileWrite>(
        batch.getInfo().imageFiles,
        [](vk::CommandBuffer submit, DeletionQueue &transientQueue, const TransferBatch::ImageFileWrite &write) {
            ImageFile imageFile = write.imageFile;
            imageFile.read();

            const ImageFile::Data &imageFileData = imageFile.getData();

            const Image::Data &imageData = write.image.getData();
            const Image::Info &imageInfo = write.image.getInfo();

            if (imageData.imageInfo.extent.width != imageFileData.meta.resolution.x ||
                imageData.imageInfo.extent.height != imageFileData.meta.resolution.y ||
                imageData.imageInfo.extent.depth != 1) {
                Logger::ERROR("[Transfer] invalid image extent.");
                return 0ul;
            }

            vk::ImageMemoryBarrier2 barrierPre = {
                .srcStageMask = vk::PipelineStageFlagBits2::eHost,
                .srcAccessMask = vk::AccessFlagBits2::eHostRead,
                .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
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
                .pImageMemoryBarriers = &barrierPre,
            });

            if (!write.image.map(submit, &transientQueue, imageFileData.image)) {
                Logger::ERROR("[Transfer] Failed to map image.");
                return 0ul;
            }

            vk::ImageMemoryBarrier2 barrierPost = {
                .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
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
                .pImageMemoryBarriers = &barrierPost,
            });

            return imageFileData.meta.size;
        });
}

void Transfer::immediate(const std::function<void(vk::CommandBuffer)> &record) {
    // prepare the command buffer
    vk::CommandBuffer immediate = nullptr;

    // create immediate buffers
    vk::CommandBufferAllocateInfo allocateInfo = {
        .commandPool = m_Command.pool,
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

template void Transfer::submit<TransferBatch::BufferWrite>(
    const std::vector<TransferBatch::BufferWrite> &,
    std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const TransferBatch::BufferWrite &)>);

template void Transfer::submit<TransferBatch::ImageWrite>(
    const std::vector<TransferBatch::ImageWrite> &,
    std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const TransferBatch::ImageWrite &)>);

template void Transfer::submit<TransferBatch::ImageFileWrite>(
    const std::vector<TransferBatch::ImageFileWrite> &,
    std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const TransferBatch::ImageFileWrite &)>);

} // namespace Physbuzz
