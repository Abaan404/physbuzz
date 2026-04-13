#pragma once

#include "../app/deletion.hpp"
#include "../ecs/system.hpp"
#include "../io/image.hpp"

namespace Physbuzz {

class TransferBatch {
  public:
    struct BufferWrite {
        const Buffer &buffer;
        std::vector<std::byte> bytes;
        std::uint64_t offset;
    };

    struct ImageWrite {
        const Image &image;
        std::vector<std::byte> bytes;
    };

    struct ImageFileWrite {
        const Image &image;
        ImageFile::Info imageFile;
    };

    struct Info {
        std::vector<BufferWrite> buffers;
        std::vector<ImageWrite> images;
        std::vector<ImageFileWrite> imageFiles;
    };

    TransferBatch(const Info &info);

    bool add(const Buffer &buffer, std::vector<std::byte> &&bytes, std::uint64_t offset);
    bool add(const Image &image, std::vector<std::byte> &&bytes);
    bool add(const Image &image, const ImageFile::Info &imageFile);

    const Info &getInfo() const;

  private:
    Info m_Info;
};

class Transfer : public System<> {
  public:
    struct Info {
        std::size_t maxChunkSize = 64 * 1024 * 1024;
    };

    Transfer(const Info &info);

    bool build() override;
    bool destroy() override;

    void submit(const TransferBatch &batch);
    void immediate(const std::function<void(vk::CommandBuffer)> &record);

    const Info getInfo() const;

  private:
    Info m_Info;

    template <typename T>
    void submit(
        const std::vector<T> &writes,
        std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const T &)> record);

    template <typename T, typename Result>
    void submit(
        const std::vector<T> &writes,
        std::function<std::optional<Result>(const T &)> prepare,
        std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const T &, const Result &)> record);

    struct {
        vk::CommandPool pool;
    } m_Command;

    struct {
        vk::Fence submit = nullptr;
        vk::Fence immediate = nullptr;
    } m_Fences;
};

extern template void Transfer::submit<TransferBatch::BufferWrite>(
    const std::vector<TransferBatch::BufferWrite> &,
    std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const TransferBatch::BufferWrite &)>);

extern template void Transfer::submit<TransferBatch::ImageWrite>(
    const std::vector<TransferBatch::ImageWrite> &,
    std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const TransferBatch::ImageWrite &)>);

extern template void Transfer::submit<TransferBatch::ImageFileWrite, ImageFile::Data>(
    const std::vector<TransferBatch::ImageFileWrite> &,
    std::function<std::optional<ImageFile::Data>(const TransferBatch::ImageFileWrite &)> prepare,
    std::function<std::size_t(vk::CommandBuffer, DeletionQueue &, const TransferBatch::ImageFileWrite &, const ImageFile::Data &)> record);

} // namespace Physbuzz
