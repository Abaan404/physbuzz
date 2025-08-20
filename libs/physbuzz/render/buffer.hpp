#pragma once

#include "../app/application.hpp"
#include <optional>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Buffer {
  public:
    using BufferUsageFlagBits = vk::BufferUsageFlagBits;
    using BufferUsageFlags = vk::BufferUsageFlags;

    using MemoryPropertyFlagBits = vk::MemoryPropertyFlagBits;
    using MemoryPropertyFlags = vk::MemoryPropertyFlags;

    using SharingMode = vk::SharingMode;

    struct Info {
        std::size_t size;
        BufferUsageFlags usage;
        MemoryPropertyFlags properties;
        SharingMode sharingMode = SharingMode::eExclusive;
    };

    struct Data {
        vk::Buffer buffer = nullptr;
        vk::DeviceMemory memory = nullptr;
    };

    template <typename T>
    bool map(const std::vector<T> &data) {
        if (m_Data.buffer == nullptr || m_Data.memory == nullptr) {
            return false;
        }

        std::span<const std::byte> bytes = {
            reinterpret_cast<const std::byte *>(data.data()),
            data.size() * sizeof(T),
        };

        void *mem = PBZ_VK_CHECK(App::Device.mapMemory(m_Data.memory, 0, bytes.size()));
        std::memcpy(mem, bytes.data(), bytes.size());
        App::Device.unmapMemory(m_Data.memory);

        return true;
    }

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Buffer(const Info &info);

    bool build();
    bool destroy();

    std::uint32_t findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    Info m_Info = {};
    Data m_Data = {};

    friend class Transfer;
};

class Transfer : public System<> {
  public:
    bool build();
    bool destroy();

    void tick();

    std::optional<Buffer> createBuffer(const Buffer::Info &info);
    bool eraseBuffer(Buffer buffer);

    bool copy(const Buffer &src, const Buffer &dst, std::size_t size, bool eraseSrc = false);

  private:
    struct {
        vk::CommandPool pool;
        vk::CommandBuffer buffer;
    } m_Command;

    struct {
        vk::Fence submit;
    } m_Fences;

    struct CopyOp {
        Buffer src;
        Buffer dst;
        vk::DeviceSize size;
        bool eraseSrc;
    };

    bool m_SubmissionInFlight = false;
    std::vector<CopyOp> m_PendingCopies;
    std::vector<Buffer> m_PendingErase;
};

} // namespace Physbuzz
