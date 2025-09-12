#pragma once

#include "../app/application.hpp"
#include "../resources/resources.hpp"

namespace Physbuzz {

class Texture;
class Uniform;
class RenderPipeline;

class PipelineLayout {
  public:
    using Type = vk::DescriptorType;

    using ShaderStage = vk::ShaderStageFlags;
    using ShaderStageFlags = vk::ShaderStageFlagBits;

    struct Binding {
        Type type;
        std::uint32_t count = 1;
        ShaderStage stage;
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

    bool allocate(const Resource<PipelineLayout> &layouts);
    bool deallocate(const Resource<PipelineLayout> &layouts);

    bool attach(const Resource<PipelineLayout> &layout, std::uint32_t binding, const Resource<Uniform> &uniform);
    bool attach(const Resource<PipelineLayout> &layout, std::uint32_t binding, const Resource<Texture> &texture);

    void reset();

    void bind(const vk::CommandBuffer &commandBuffer, const Resource<RenderPipeline> &pipeline);

  private:
    vk::DescriptorPool createPool();

    struct Allocation {
        vk::DescriptorPool allocatorPool = nullptr;
        std::vector<vk::DescriptorSet> sets;
    };

    std::unordered_map<Resource<PipelineLayout>, Allocation> m_AllocatedLayouts;

    Info m_Info;

    vk::DescriptorPool m_CurrentPool = nullptr;
    std::vector<vk::DescriptorPool> m_UsedPools;
    std::vector<vk::DescriptorPool> m_FreePools;
};

} // namespace Physbuzz
