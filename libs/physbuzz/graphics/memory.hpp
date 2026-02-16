#pragma once

#include <glm/glm.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class DeletionQueue;

class Buffer {
  public:
    using UsageFlagBits = vk::BufferUsageFlagBits;
    using UsageFlags = vk::BufferUsageFlags;
    using SharingMode = vk::SharingMode;

    using FlagBits = vk::BufferCreateFlagBits;
    using Flags = vk::BufferCreateFlags;

    enum class MemoryUsage {
        Auto,
        CPUOnly,
        GPUOnly,
        CPUToGPU,
        GPUToCPU
    };

    struct Info {
        UsageFlags usage;
        Flags flags = {};
        MemoryUsage memoryUsage = MemoryUsage::Auto;
        SharingMode sharingMode = SharingMode::eExclusive;
    };

    struct Data {
        vk::Buffer buffer = nullptr;
        vk::BufferCreateInfo bufferInfo = {};
    };

    Buffer(const Info &info);

    bool build(std::uint64_t size);
    bool destroy();

    bool map(vk::CommandBuffer cmd, DeletionQueue *deletion, const std::span<const std::byte> &bytes, std::uint64_t offset) const;

    bool copy(vk::CommandBuffer cmd, const Buffer &src, const std::vector<vk::BufferCopy> &copies) const;

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    VmaAllocation m_Allocation = nullptr;

    friend class Image;
};

class Image {
  public:
    using UsageFlagBits = vk::ImageUsageFlagBits;
    using UsageFlags = vk::ImageUsageFlags;
    using SharingMode = vk::SharingMode;

    using Type = vk::ImageType;
    using Format = vk::Format;

    using FlagBits = vk::ImageCreateFlagBits;
    using Flags = vk::ImageCreateFlags;

    enum class MemoryUsage {
        Auto,
        CPUOnly,
        GPUOnly,
        CPUToGPU,
        GPUToCPU
    };

    struct Info {
        UsageFlags usage;
        Type type;
        std::uint32_t mipLevels = 1;
        std::uint32_t arrayLayers = 1;
        Flags flags = {};
        Format format = Format::eR8G8B8A8Unorm;
        MemoryUsage memoryUsage = MemoryUsage::Auto;
        SharingMode sharingMode = SharingMode::eExclusive;
    };

    struct Data {
        vk::Image image = nullptr;
        vk::ImageCreateInfo imageInfo = {};
    };

    Image(const Info &info);

    bool build(const glm::uvec3 &extent);
    bool destroy();

    bool map(vk::CommandBuffer cmd, DeletionQueue *deletion, const std::span<const std::byte> &bytes, vk::ImageLayout layout) const;

    bool copy(vk::CommandBuffer cmd, const Buffer &src, const std::vector<vk::BufferImageCopy> &copies, vk::ImageLayout layout) const;
    bool copy(vk::CommandBuffer cmd, const Image &src, const std::vector<vk::ImageCopy> &copies, vk::ImageLayout layout) const;

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    VmaAllocation m_Allocation = nullptr;

    friend class Buffer;
};

} // namespace Physbuzz
