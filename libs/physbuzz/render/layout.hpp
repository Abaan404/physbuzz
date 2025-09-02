#pragma once

#include "../app/application.hpp"
#include "buffer.hpp"
#include "renderer.hpp"

namespace Physbuzz {

class PipelineLayout {
  public:
    using Type = vk::DescriptorType;

    using ShaderStage = vk::ShaderStageFlags;
    using ShaderStageFlags = vk::ShaderStageFlagBits;

    struct Binding {
        std::uint32_t size;
        std::uint32_t count = 1;
        Type type;
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
    struct PoolSize {
        vk::DescriptorType type;
        float multiplier;
    };

    struct Info {
        uint32_t chunkSize = 100;
        std::vector<PoolSize> poolSizes = {
            {vk::DescriptorType::eSampler, 0.5f},
            {vk::DescriptorType::eCombinedImageSampler, 4.f},
            {vk::DescriptorType::eSampledImage, 4.f},
            {vk::DescriptorType::eStorageImage, 1.f},
            {vk::DescriptorType::eUniformBuffer, 2.f},
            {vk::DescriptorType::eStorageBuffer, 2.f},
            {vk::DescriptorType::eInputAttachment, 0.5f},
        };
    };

    PipelineLayoutAllocator(const Info &info);

    bool build();
    bool destroy();

    bool allocate(const Resource<PipelineLayout> &layouts);
    bool deallocate(const Resource<PipelineLayout> &layouts);

    template <typename T>
    bool update(const Resource<PipelineLayout> &layout, std::uint32_t binding, const std::vector<T> &data) {
        if (!m_AllocatedLayouts.contains(layout)) {
            Logger::DEBUG("[PipelineLayoutAllocator] Allocating layout \"{}\"", layout.getIdentifier());
            if (!allocate(layout)) {
                return false;
            }
        }

        PBZ_ASSERT(binding < layout->getInfo().bindings.size(), "[PipelineLayoutAllocator] Invalid binding to update.");
        PBZ_ASSERT(sizeof(T) == layout->getInfo().bindings[binding].size, "[PipelineLayoutAllocator] Invalid stride for layout to update.");
        PBZ_ASSERT(data.size() <= layout->getInfo().bindings[binding].count, "[PipelineLayoutAllocator] Too much data to update.");

        const std::shared_ptr<Renderer> renderer = m_Scene->getSystem<Renderer>();
        const std::shared_ptr<Transfer> transfer = m_Scene->getSystem<Transfer>();

        return transfer->map(m_AllocatedLayouts[layout].buffers[renderer->m_Frame.inFlight + binding], data);
    }

    void reset();

    void bind(const vk::CommandBuffer &commandBuffer, const Resource<RenderPipeline> &pipeline);

  private:
    vk::DescriptorPool createPool();

    struct Allocation {
        vk::DescriptorPool allocatorPool = nullptr;
        std::vector<vk::DescriptorSet> sets;
        std::vector<Buffer> buffers;
    };

    std::unordered_map<Resource<PipelineLayout>, Allocation> m_AllocatedLayouts;

    Info m_Info;

    vk::DescriptorPool m_CurrentPool = nullptr;
    std::vector<vk::DescriptorPool> m_UsedPools;
    std::vector<vk::DescriptorPool> m_FreePools;
};

} // namespace Physbuzz
