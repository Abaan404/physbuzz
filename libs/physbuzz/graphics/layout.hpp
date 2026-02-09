#pragma once

#include "../ecs/system.hpp"
#include "../resources/resource.hpp"
#include "defines.hpp"

namespace Physbuzz {

class Buffer;
class DynamicBuffer;
class Texture;
class RenderPipeline;
class Renderer;

class PipelineLayout {
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

    PipelineLayout(const Info &info);

    bool build();
    bool destroy();

    const Info &getInfo() const;

  private:
    Info m_Info;

    std::uint32_t m_DynamicOffset = 0;
    vk::DescriptorSetLayout m_Layout = nullptr;

    friend class RenderPipeline;
    friend class PipelineLayoutAllocator;
};

template <>
struct IsResource<PipelineLayout> : std::true_type {};

class PipelineLayoutAllocator : public System<> {
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

    PipelineLayoutAllocator(const Info &info);

    bool build();
    bool destroy();

    bool write(const Resource<PipelineLayout> &layout, const Resource<DynamicBuffer> &buffer, std::uint32_t binding, std::uint32_t element = 0);
    bool write(const Resource<PipelineLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding, std::uint32_t element = 0);

    void reset();

    void bind(const RenderContext &context, const Resource<RenderPipeline> &pipeline, std::uint32_t idx = 0);

  private:
    struct Allocation {
        vk::DescriptorPool allocatorPool = nullptr;
        std::vector<vk::DescriptorSet> sets;
    };

    struct WriteInfo {
        std::uint32_t binding;
        std::uint32_t element;
    };

    struct WriteEntry {
        EventID resize = -1;
        std::unordered_map<Resource<PipelineLayout>, WriteInfo> layouts;
    };

    bool rewrite(const Resource<PipelineLayout> &layout, const DynamicBuffer *buffer, const RenderContext &context, std::uint32_t binding, std::uint32_t element = 0);

    bool allocate(const Resource<PipelineLayout> &layout);
    bool deallocate(const Resource<PipelineLayout> &layout);

    vk::DescriptorPool createPool();

    Info m_Info;

    std::unordered_map<DynamicBuffer *, WriteEntry> m_WrittenBuffers;

    std::unordered_map<Resource<PipelineLayout>, Allocation> m_AllocatedLayouts;

    vk::DescriptorPool m_CurrentPool = nullptr;
    std::vector<vk::DescriptorPool> m_UsedPools;
    std::vector<vk::DescriptorPool> m_FreePools;
};

} // namespace Physbuzz
