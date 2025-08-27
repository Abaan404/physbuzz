#include "layout.hpp"

namespace Physbuzz {

PipelineLayout::PipelineLayout(const Info &info)
    : m_Info(info) {}

bool PipelineLayout::build() {
    if (m_Layout != nullptr) {
        Logger::WARNING("[PipelineLayout] Trying to build a constructed descriptor.");
        return true;
    }

    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.resize(m_Info.bindings.size());

    for (std::uint32_t i = 0; i < m_Info.bindings.size(); i++) {
        layoutBindings[i] = {
            .binding = i,
            .descriptorType = m_Info.bindings[i].type,
            .descriptorCount = m_Info.bindings[i].count,
            .stageFlags = m_Info.bindings[i].stage,
        };
    }

    m_Layout = PBZ_VK_CHECK(App::Device.createDescriptorSetLayout({
        .bindingCount = static_cast<std::uint32_t>(layoutBindings.size()),
        .pBindings = layoutBindings.data(),
    }));

    return true;
}

bool PipelineLayout::destroy() {
    if (m_Layout == nullptr) {
        Logger::WARNING("[PipelineLayout] Trying to destroy a destructed descriptor.");
        return false;
    }

    App::Device.destroyDescriptorSetLayout(m_Layout);
    m_Layout = nullptr;

    return true;
}

const PipelineLayout::Info &PipelineLayout::getInfo() const {
    return m_Info;
}

PipelineLayoutAllocator::PipelineLayoutAllocator(const Info &info)
    : m_Info(info) {}

bool PipelineLayoutAllocator::build() {
    if (m_CurrentPool != nullptr) {
        Logger::WARNING("[PipelineLayoutAllocator] Trying to build a constructed pool allocator.");
        return false;
    }

    m_CurrentPool = createPool();
    return true;
}

bool PipelineLayoutAllocator::destroy() {
    if (m_CurrentPool == nullptr) {
        Logger::WARNING("[PipelineLayoutAllocator] Trying to destroy a destructed pool allocator.");
        return false;
    }

    App::Device.destroyDescriptorPool(m_CurrentPool);
    m_CurrentPool = nullptr;

    for (auto &pool : m_UsedPools) {
        App::Device.destroyDescriptorPool(pool);
    }

    for (auto &pool : m_FreePools) {
        App::Device.destroyDescriptorPool(pool);
    }

    m_UsedPools.clear();
    m_FreePools.clear();

    return true;
}

bool PipelineLayoutAllocator::allocate(const Resource<PipelineLayout> &layout) {
    if (m_AllocatedLayouts.contains(layout)) {
        Logger::WARNING("[PipelineLayoutAllocator] Trying to allocate an allocated layout.");
        return true;
    }

    std::vector<vk::DescriptorSetLayout> setLayouts(Renderer::Frames::MAX_IN_FLIGHT, layout->m_Layout);

    vk::DescriptorSetAllocateInfo allocateInfo = {
        .descriptorPool = m_CurrentPool,
        .descriptorSetCount = Renderer::Frames::MAX_IN_FLIGHT,
        .pSetLayouts = setLayouts.data(),
    };

    auto [result, sets] = App::Device.allocateDescriptorSets(allocateInfo);

    switch (result) {
    case vk::Result::eSuccess:
        break;

    case vk::Result::eErrorOutOfPoolMemory:
    case vk::Result::eErrorFragmentation:
        // move current pool to used and make new one
        if (!m_FreePools.empty()) {
            m_CurrentPool = m_FreePools.back();
            m_FreePools.pop_back();
        } else {
            m_UsedPools.push_back(m_CurrentPool);
            m_CurrentPool = createPool();
        }

        allocateInfo.descriptorPool = m_CurrentPool;
        sets = PBZ_VK_CHECK(App::Device.allocateDescriptorSets(allocateInfo));
        break;

    default:
        Logger::ERROR("[PipelineLayoutAllocator] Failed to allocate PipelineLayout sets.");
        return false;
    }

    std::shared_ptr<Transfer> transfer = m_Scene->getSystem<Transfer>();
    std::vector<Buffer> buffers;

    const auto &layoutBindings = layout->getInfo().bindings;

    for (std::size_t i = 0; i < layoutBindings.size(); i++) {
        const auto &binding = layoutBindings[i];

        switch (binding.type) {
        case vk::DescriptorType::eUniformBuffer: {
            std::optional<Buffer> buffer = transfer->createBuffer({
                .size = binding.size * binding.count,
                .usage = vk::BufferUsageFlagBits::eUniformBuffer,
                .properties = vk::MemoryPropertyFlagBits::eHostVisible |
                              vk::MemoryPropertyFlagBits::eHostCoherent,
            });

            if (!buffer) {
                Logger::ERROR("[PipelineLayoutAllocator] Failed to create uniform buffer.");
                App::Device.freeDescriptorSets(m_CurrentPool, sets);
                continue;
            }

            buffers.push_back(*buffer);

            for (uint32_t frame = 0; frame < Renderer::Frames::MAX_IN_FLIGHT; frame++) {
                vk::DescriptorBufferInfo bufferInfo = {
                    .buffer = buffer->getData().buffer,
                    .offset = 0,
                    .range = binding.size * binding.count,
                };

                vk::WriteDescriptorSet write = {
                    .dstSet = sets[frame],
                    .dstBinding = static_cast<std::uint32_t>(i),
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo = &bufferInfo,
                };

                App::Device.updateDescriptorSets(write, {});
            }
        } break;

        default:
            Logger::ERROR("[PipelineLayoutAllocator] Unsupported PipelineLayout type.");
            return false;
        }
    }

    m_AllocatedLayouts[layout] = {
        .allocatorPool = m_CurrentPool,
        .sets = sets,
        .buffers = buffers,
    };

    return true;
}

bool PipelineLayoutAllocator::deallocate(const Resource<PipelineLayout> &layout) {
    if (!m_AllocatedLayouts.contains(layout)) {
        return false;
    }

    const Allocation &alloc = m_AllocatedLayouts[layout];
    App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets);

    return true;
}

void PipelineLayoutAllocator::reset() {
    // free all allocated sets, not necessary since we're resetting the pool, however since we're no longer tracking them we might aswell.
    for (const auto &[layout, alloc] : m_AllocatedLayouts) {
        App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets);
    }

    for (const auto &pool : m_UsedPools) {
        App::Device.resetDescriptorPool(pool);
    }

    m_AllocatedLayouts.clear();

    m_FreePools.insert(m_FreePools.end(), std::make_move_iterator(m_UsedPools.begin()), std::make_move_iterator(m_UsedPools.end()));
    m_UsedPools.clear();
}

void PipelineLayoutAllocator::bind(const vk::CommandBuffer &commandBuffer, const Resource<RenderPipeline> &pipeline) {
    const std::shared_ptr<Renderer> renderer = m_Scene->getSystem<Renderer>();

    for (const auto &layout : pipeline->getInfo().layouts) {
        if (!m_AllocatedLayouts.contains(layout)) {
            Logger::DEBUG("[PipelineLayoutAllocator] Allocating layout \"{}\" for pipeline \"{}\"", layout.getIdentifier(), pipeline.getIdentifier());
            if (!allocate(layout)) {
                Logger::CRITICAL("[PipelineLayoutAllocator] Could not allocate layout to bind PipelineLayout.");
                continue;
            }
        }

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->m_Layout, 0, m_AllocatedLayouts[layout].sets[renderer->m_Frame.inFlight], nullptr);
    }
}

vk::DescriptorPool PipelineLayoutAllocator::createPool() {
    std::vector<vk::DescriptorPoolSize> sizes;
    for (auto &[type, multiplier] : m_Info.poolSizes) {
        sizes.push_back({
            .type = type,
            .descriptorCount = Renderer::Frames::MAX_IN_FLIGHT * static_cast<std::uint32_t>(m_Info.chunkSize * multiplier),
        });
    }

    return PBZ_VK_CHECK(App::Device.createDescriptorPool({
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = m_Info.chunkSize * Renderer::Frames::MAX_IN_FLIGHT,
        .poolSizeCount = static_cast<std::uint32_t>(sizes.size()),
        .pPoolSizes = sizes.data(),
    }));
}

} // namespace Physbuzz
