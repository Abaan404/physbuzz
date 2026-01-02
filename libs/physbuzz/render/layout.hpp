#pragma once

#include "../app/application.hpp"
#include "../resources/resources.hpp"
#include "renderers/defines.hpp"

namespace Physbuzz {

class Buffer;
class ShaderBuffer;
class Texture;
class RenderPipeline;
class Renderer;

class PipelineLayout {
  public:
    using Type = vk::DescriptorType;

    using ShaderStage = vk::ShaderStageFlags;
    using ShaderStageFlags = vk::ShaderStageFlagBits;

    struct Binding {
        Type type;
        ShaderStage stage = ShaderStageFlags::eAll;
        std::uint32_t count = 1;
        std::uint64_t offset = 0;
        std::uint32_t range;
    };

    struct Info {
        std::vector<Binding> bindings;
    };

    PipelineLayout(const Info &info);

    bool build();
    bool destroy();

    const Info &getInfo() const;

  private:
    Info m_Info;

    std::uint32_t m_DynamicOffset;
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

    bool allocate(const Resource<PipelineLayout> &layout);
    bool deallocate(const Resource<PipelineLayout> &layout);

    bool attach(const Resource<PipelineLayout> &layout, const Resource<ShaderBuffer> &storage, std::uint32_t binding);
    bool attach(const Resource<PipelineLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding);

    bool attach(const RenderContext &context, const Resource<PipelineLayout> &layout, const Resource<ShaderBuffer> &storage, std::uint32_t binding);
    bool attach(const RenderContext &context, const Resource<PipelineLayout> &layout, const Resource<Texture> &storage, std::uint32_t binding);

    void reset();

    void bind(const RenderContext &context, const Resource<RenderPipeline> &pipeline, std::uint32_t idx = 0);

  private:
    vk::DescriptorPool createPool();

    struct Allocation {
        vk::DescriptorPool allocatorPool = nullptr;
        std::vector<vk::DescriptorSet> sets;
    };

    std::unordered_map<Resource<PipelineLayout>, Allocation> m_AllocatedLayouts;
    std::unordered_map<Resource<PipelineLayout>, std::uint32_t> m_AttachedCount;

    Info m_Info;

    vk::DescriptorPool m_CurrentPool = nullptr;
    std::vector<vk::DescriptorPool> m_UsedPools;
    std::vector<vk::DescriptorPool> m_FreePools;
};

} // namespace Physbuzz
