#pragma once

#include "../resources/resource.hpp"
#include "defines.hpp"

namespace Physbuzz {

class Buffer;
class DynamicBuffer;
class Texture;
class Sampler;
class Attachment;

class DescriptorLayout {
  public:
    using Type = vk::DescriptorType;
    using Flags = vk::DescriptorSetLayoutCreateFlagBits;

    using ShaderStage = vk::ShaderStageFlags;
    using ShaderStageFlags = vk::ShaderStageFlagBits;
    using BindingFlags = vk::DescriptorBindingFlags;
    using BindingFlagBits = vk::DescriptorBindingFlagBits;

    struct Binding {
        Type type;
        BindingFlags flags = {};
        ShaderStage stage = ShaderStageFlags::eAll;
        std::uint32_t count = 1;
        std::uint64_t offset = 0;
        std::uint64_t range = vk::WholeSize;
    };

    enum class Lifetime {
        Global,
        PerFrame,
    };

    struct Info {
        std::vector<Binding> bindings;
        Flags flags = {};
        Lifetime lifetime = Lifetime::PerFrame;
    };

    struct Data {
        vk::DescriptorSetLayout layout = nullptr;
    };

    DescriptorLayout(const Info &info);

    bool build();
    bool destroy();

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info;
    Data m_Data;
};

template <>
struct IsResource<DescriptorLayout> : std::true_type {};

class DescriptorLayoutAllocator {
  public:
    using Type = vk::DescriptorType;

    struct PoolSize {
        Type type;
        float multiplier;
    };

    struct Info {
        uint32_t chunkSize = 100;
        std::vector<PoolSize> poolSizes = {
            {Type::eSampler, 0.5f},
            {Type::eCombinedImageSampler, 4.f},
            {Type::eSampledImage, 4.f},
            {Type::eStorageImage, 1.f},
            {Type::eUniformBuffer, 2.f},
            {Type::eStorageBuffer, 2.f},
            {Type::eInputAttachment, 0.5f},
        };
    };

    DescriptorLayoutAllocator(const Info &info);

    bool build();
    bool destroy();

    bool write(const Resource<DescriptorLayout> &layout, const Resource<DynamicBuffer> &buffer, std::uint32_t binding, std::uint32_t element = 0);
    bool write(const Resource<DescriptorLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding, std::uint32_t element = 0);
    bool write(const Resource<DescriptorLayout> &layout, const Resource<Attachment> &attachment, std::uint32_t binding, std::uint32_t element = 0);
    bool write(const Resource<DescriptorLayout> &layout, const Resource<Sampler> &sampler, std::uint32_t binding, std::uint32_t element = 0);

    void reset();

    void bind(const RenderContext &context, const GraphicsPipeline &pipeline, std::uint32_t idx = 0);
    void bind(const RenderContext &context, const ComputePipeline &pipeline, std::uint32_t idx = 0);

  private:
    struct Allocation {
        vk::DescriptorPool allocatorPool = nullptr;
        std::vector<vk::DescriptorSet> sets;
    };

    struct WriteInfo {
        EventID rebuild = -1;
        std::uint32_t binding;
        std::uint32_t element;
    };

    struct WriteEntry {
        std::unordered_map<Resource<DescriptorLayout>, WriteInfo> layouts;
    };

    bool rewrite(const Resource<DescriptorLayout> &layout, const Resource<DynamicBuffer> &buffer, std::uint32_t frameInFlight, std::uint32_t binding, std::uint32_t element = 0);
    bool rewrite(const Resource<DescriptorLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding, std::uint32_t element = 0);
    bool rewrite(const Resource<DescriptorLayout> &layout, const Resource<Attachment> &attachment, std::uint32_t frameInFlight, std::uint32_t binding, std::uint32_t element = 0);

    bool allocate(const Resource<DescriptorLayout> &layout);
    bool deallocate(const Resource<DescriptorLayout> &layout);

    vk::DescriptorPool createPool();

    Info m_Info;

    std::unordered_map<Resource<DynamicBuffer>, WriteEntry> m_WrittenBuffers;
    std::unordered_map<Resource<Texture>, WriteEntry> m_WrittenTextures;
    std::unordered_map<Resource<Attachment>, WriteEntry> m_WrittenAttachments;

    std::unordered_map<Resource<DescriptorLayout>, Allocation> m_AllocatedLayouts;

    vk::DescriptorPool m_CurrentPool = nullptr;
    std::vector<vk::DescriptorPool> m_UsedPools;
    std::vector<vk::DescriptorPool> m_FreePools;
};

} // namespace Physbuzz
