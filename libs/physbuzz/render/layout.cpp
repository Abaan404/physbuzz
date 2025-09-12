#include "layout.hpp"

#include "renderers/defines.hpp"
#include "textures/texture.hpp"
#include "uniform.hpp"

namespace Physbuzz {

PipelineLayout::PipelineLayout(const Info &info)
    : m_Info(info) {}

bool PipelineLayout::build() {
    if (m_Layout != nullptr) {
        Logger::WARNING("[PipelineLayout] Trying to build a constructed descriptor.");
        return true;
    }

    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(m_Info.bindings.size());

    for (std::uint32_t i = 0; i < m_Info.bindings.size(); i++) {
        layoutBindings.emplace_back<vk::DescriptorSetLayoutBinding>({
            .binding = i,
            .descriptorType = m_Info.bindings[i].type,
            .descriptorCount = m_Info.bindings[i].count,
            .stageFlags = m_Info.bindings[i].stage,
        });
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

    for (auto [layout, alloc] : m_AllocatedLayouts) {
        App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets);
    }

    m_AllocatedLayouts.clear();

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

    std::vector<vk::DescriptorSetLayout> setLayouts(detail::MAX_FRAMES_IN_FLIGHT, layout->m_Layout);

    vk::DescriptorSetAllocateInfo allocateInfo = {
        .descriptorPool = m_CurrentPool,
        .descriptorSetCount = detail::MAX_FRAMES_IN_FLIGHT,
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

    m_AllocatedLayouts[layout] = {
        .allocatorPool = m_CurrentPool,
        .sets = sets,
    };

    return true;
}

bool PipelineLayoutAllocator::deallocate(const Resource<PipelineLayout> &layout) {
    if (!m_AllocatedLayouts.contains(layout)) {
        return false;
    }

    Allocation &alloc = m_AllocatedLayouts[layout];
    App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets);

    m_AllocatedLayouts.erase(layout);
    return true;
}

bool PipelineLayoutAllocator::attach(const Resource<PipelineLayout> &layout, std::uint32_t binding, const Resource<Uniform> &uniform) {
    if (layout->getInfo().bindings[binding].type != vk::DescriptorType::eUniformBuffer) {
        Logger::ERROR("[PipelineLayoutAllocator] Invalid type at binding {} for resource \"{}\"", binding, layout.getIdentifier());
        return false;
    }

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    const std::vector<Buffer> &buffers = uniform->getBuffers();

    for (uint32_t frame = 0; frame < detail::MAX_FRAMES_IN_FLIGHT; frame++) {
        vk::DescriptorBufferInfo bufferInfo = {
            .buffer = buffers[frame].getData().buffer,
            .offset = 0,
            .range = uniform->getRange(),
        };

        vk::WriteDescriptorSet write = {
            .dstSet = m_AllocatedLayouts[layout].sets[frame],
            .dstBinding = static_cast<std::uint32_t>(binding),
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo,
        };

        App::Device.updateDescriptorSets(write, {});
    }

    return true;
}

bool PipelineLayoutAllocator::attach(const Resource<PipelineLayout> &layout, std::uint32_t binding, const Resource<Texture> &texture) {
    if (layout->getInfo().bindings[binding].type != vk::DescriptorType::eCombinedImageSampler) {
        Logger::ERROR("[PipelineLayoutAllocator] Invalid type at binding {} for resource \"{}\"", binding, layout.getIdentifier());
        return false;
    }

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    vk::DescriptorImageInfo imageInfo = {
        .sampler = texture->getSampler(),
        .imageView = texture->getImageView(),
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
    };

    std::vector<vk::WriteDescriptorSet> writes;
    for (uint32_t frame = 0; frame < detail::MAX_FRAMES_IN_FLIGHT; frame++) {
        writes.emplace_back<vk::WriteDescriptorSet>({
            .dstSet = m_AllocatedLayouts[layout].sets[frame],
            .dstBinding = static_cast<std::uint32_t>(binding),
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &imageInfo,
        });
    }

    App::Device.updateDescriptorSets(writes, {});
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

    for (const auto &pool : m_FreePools) {
        App::Device.resetDescriptorPool(pool);
    }

    m_FreePools.insert(m_FreePools.end(), std::make_move_iterator(m_UsedPools.begin()), std::make_move_iterator(m_UsedPools.end()));
    m_UsedPools.clear();
    m_AllocatedLayouts.clear();
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

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->m_Layout, 0, m_AllocatedLayouts[layout].sets[renderer->getFrameInFlight()], nullptr);
    }
}

vk::DescriptorPool PipelineLayoutAllocator::createPool() {
    std::vector<vk::DescriptorPoolSize> sizes;
    for (auto &[type, multiplier] : m_Info.poolSizes) {
        sizes.push_back({
            .type = type,
            .descriptorCount = detail::MAX_FRAMES_IN_FLIGHT * static_cast<std::uint32_t>(m_Info.chunkSize * multiplier),
        });
    }

    return PBZ_VK_CHECK(App::Device.createDescriptorPool({
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = m_Info.chunkSize * detail::MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = static_cast<std::uint32_t>(sizes.size()),
        .pPoolSizes = sizes.data(),
    }));
}

} // namespace Physbuzz
