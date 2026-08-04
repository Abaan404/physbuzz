#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class DeletionQueue;

class Buffer {
  public:
    using Usage = vk::BufferUsageFlags;
    using UsageFlags = vk::BufferUsageFlagBits;
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
        Usage usage;
        Flags flags;
        MemoryUsage memoryUsage = MemoryUsage::Auto;
        SharingMode sharingMode = SharingMode::eExclusive;
    };

    struct Data {
        vk::Buffer buffer = nullptr;
        vk::BufferCreateInfo bufferInfo = {};
        vk::DeviceAddress address = 0;
    };

    Buffer(const Info &info);

    bool build(std::uint64_t size);
    bool destroy();

    bool map(vk::CommandBuffer cmd, DeletionQueue *deletion, const std::span<const std::byte> &bytes, std::uint64_t offset) const;

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
    using Usage = vk::ImageUsageFlags;
    using UsageFlags = vk::ImageUsageFlagBits;
    using SharingMode = vk::SharingMode;

    using AspectFlags = vk::ImageAspectFlagBits;
    using Aspect = vk::ImageAspectFlags;

    using Type = vk::ImageType;
    using Format = vk::Format;

    using FlagBits = vk::ImageCreateFlagBits;
    using Flags = vk::ImageCreateFlags;

    using ViewType = vk::ImageViewType;
    using SubresourceRange = vk::ImageSubresourceRange;

    static constexpr std::uint32_t RemainingMipLevels = vk::RemainingMipLevels;
    static constexpr std::uint32_t RemainingArrayLayers = vk::RemainingArrayLayers;

    enum class MemoryUsage {
        Auto,
        CPUOnly,
        GPUOnly,
        CPUToGPU,
        GPUToCPU
    };

    struct ViewInfo {
        ViewType type = ViewType::e2D;
        SubresourceRange subresourceRange = {
            .aspectMask = {},
            .baseMipLevel = 0,
            .levelCount = 0,
            .baseArrayLayer = 0,
            .layerCount = 0,
        };

        bool operator==(const ViewInfo &other) const;
    };

    struct Info {
        Usage usage;
        Type type;
        std::uint32_t mipLevels = 1;
        std::uint32_t arrayLayers = 1;
        Flags flags = {};
        Format format = Format::eR16G16B16A16Sfloat;
        MemoryUsage memoryUsage = MemoryUsage::Auto;
        SharingMode sharingMode = SharingMode::eExclusive;
        std::vector<ViewInfo> views;
    };

    struct ViewInfoHash {
        std::size_t operator()(const ViewInfo &info) const;
    };

    struct Data {
        vk::Image image = nullptr;
        vk::ImageCreateInfo imageInfo = {};
        std::unordered_map<ViewInfo, vk::ImageView, ViewInfoHash> views = {};
    };

    Image(const Info &info);

    bool build(const glm::uvec3 &extent);
    bool destroy();

    bool map(vk::CommandBuffer cmd, DeletionQueue *deletion, const std::span<const std::byte> &bytes) const;

    bool buildView(const ViewInfo &info);
    bool destroyView(const ViewInfo &info);

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    VmaAllocation m_Allocation = nullptr;

    friend class Buffer;
};

} // namespace Physbuzz
