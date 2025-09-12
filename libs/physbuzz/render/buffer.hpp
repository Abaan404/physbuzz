#pragma once

#include "../app/application.hpp"

namespace Physbuzz {

class Transfer;

class Buffer {
  public:
    using BufferUsageFlagBits = vk::BufferUsageFlagBits;
    using BufferUsageFlags = vk::BufferUsageFlags;
    using SharingMode = vk::SharingMode;

    enum class MemoryUsage {
        Auto,
        CPUOnly,
        GPUOnly,
        CPUToGPU,
        GPUToCPU
    };

    struct Info {
        BufferUsageFlags usage;
        MemoryUsage memoryUsage = MemoryUsage::Auto;
        SharingMode sharingMode = SharingMode::eExclusive;
    };

    struct Data {
        vk::Buffer buffer = nullptr;
        vk::BufferCreateInfo bufferInfo = {};
    };

    Buffer(const Info &info);

    bool build(std::size_t size);
    bool destroy();

    bool map(const std::span<const std::byte> &data) const;
    void copy(const vk::CommandBuffer &commandBuffer, const Buffer &srcBuffer) const;

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    VmaAllocation m_Allocation = nullptr;

    friend Transfer;
};

class Image {
  public:
    using ImageUsageFlagBits = vk::ImageUsageFlagBits;
    using ImageUsageFlags = vk::ImageUsageFlags;
    using SharingMode = vk::SharingMode;

    using Type = vk::ImageType;
    using Format = vk::Format;

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
        std::uint32_t mipLevels;
        std::uint32_t arrayLayers;
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

    void copy(const vk::CommandBuffer &commandBuffer, const Buffer &srcBuffer) const;

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    VmaAllocation m_Allocation = nullptr;

    friend Transfer;
};

class Transfer : public System<> {
  public:
    bool build();
    bool destroy();

    bool map(const Buffer &buffer, const std::span<const std::byte> &bytes);
    bool map(const Image &image, const std::span<const std::byte> &bytes);

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
