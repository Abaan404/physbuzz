#pragma once

#include "../ecs/system.hpp"
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Transfer;

class Buffer {
  public:
    using BufferUsageFlagBits = vk::BufferUsageFlagBits;
    using BufferUsageFlags = vk::BufferUsageFlags;
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
        BufferUsageFlags usage;
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

    bool map(const std::span<const std::byte> &data, std::uint64_t offset) const;
    bool copy(const vk::CommandBuffer &commandBuffer, const Buffer &src, std::vector<vk::BufferCopy> copies) const;

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    VmaAllocation m_Allocation = nullptr;

    friend class Transfer;
    friend class Image;
};

class Image {
  public:
    using ImageUsageFlagBits = vk::ImageUsageFlagBits;
    using ImageUsageFlags = vk::ImageUsageFlags;
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
        ImageUsageFlags usage;
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

    bool copy(const vk::CommandBuffer &commandBuffer, const Buffer &src) const;

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    VmaAllocation m_Allocation = nullptr;

    friend class Transfer;
    friend class Buffer;
};

class Transfer : public System<> {
  public:
    bool build();
    bool destroy();

    bool map(const Buffer &buffer, const std::span<const std::byte> &bytes, std::uint64_t offset);
    bool map(const Image &image, const std::span<const std::byte> &bytes);
    void immediate(std::function<void(const vk::CommandBuffer &)> record);

  private:
    struct {
        vk::CommandPool pool;
        vk::CommandBuffer buffer;
    } m_Command;

    struct {
        vk::Fence submit;
    } m_Fences;
};

} // namespace Physbuzz
