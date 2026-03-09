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
    Transfer();

    bool build() override;
    bool destroy() override;

    void submit(const TransferBatch &batch);
    void immediate(std::function<void(vk::CommandBuffer)> record);

  private:
    DeletionQueue m_Deletion;

    struct {
        vk::CommandPool pool;
    } m_Command;

    struct {
        vk::Fence submit = nullptr;
        vk::Fence immediate = nullptr;
    } m_Fences;
};

} // namespace Physbuzz
