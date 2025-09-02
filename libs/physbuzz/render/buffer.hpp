#pragma once

#include "../app/application.hpp"
#include <vulkan/vulkan.hpp>

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
        BufferUsageFlags bufferUsage;
        MemoryUsage memoryUsage = MemoryUsage::Auto;
        SharingMode sharingMode = SharingMode::eExclusive;
    };

    struct Data {
        vk::Buffer buffer = nullptr;
        vk::BufferCreateInfo bufferInfo = {};
        vk::AccessFlags accessMask = {};
        vk::PipelineStageFlags stageMask = {};
        VmaAllocation allocation = nullptr;
    };

    Buffer(const Info &info);

    bool build(std::size_t size);
    bool destroy();

    const Info &getInfo() const;
    const Data &getData() const;

    bool mapBytes(const vk::CommandBuffer &commandBuffer, const std::span<const std::byte> &data) const;

  private:
    Info m_Info = {};
    Data m_Data = {};

    friend Transfer;
};

class Transfer : public System<> {
  public:
    bool build();
    bool destroy();

    template <typename T>
    bool map(const Buffer &buffer, const std::vector<T> &data) {
        std::span<const std::byte> bytes = {
            reinterpret_cast<const std::byte *>(data.data()),
            data.size() * sizeof(T),
        };

        return mapBytes(buffer, bytes);
    }

  private:
    bool mapBytes(const Buffer &buffer, const std::span<const std::byte> &data);

    struct {
        vk::CommandPool pool;
        vk::CommandBuffer buffer;
    } m_Command;

    struct {
        vk::Fence submit;
    } m_Fences;
};

} // namespace Physbuzz
