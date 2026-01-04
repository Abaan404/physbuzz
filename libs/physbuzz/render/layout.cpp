#include "layout.hpp"

#include "layouts/shaderbuffer.hpp"
#include "layouts/texture.hpp"
#include "physbuzz/debug/macros.hpp"
#include "physbuzz/render/renderers/defines.hpp"
#include "shaders.hpp"

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

    std::array<vk::DescriptorSetLayout, detail::MAX_FRAMES_IN_FLIGHT> setLayouts;
    setLayouts.fill(layout->m_Layout);

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

bool PipelineLayoutAllocator::attach(const Resource<PipelineLayout> &layout, const Resource<ShaderBuffer> &storage, std::uint32_t binding) {
    vk::DescriptorType type;

    switch (storage->getType()) {
    case ShaderBuffer::Type::Constant:
        type = vk::DescriptorType::eUniformBuffer;
        break;

    case ShaderBuffer::Type::Structured:
        type = vk::DescriptorType::eStorageBuffer;
        break;

    case ShaderBuffer::Type::ConstantDynamic:
        type = vk::DescriptorType::eUniformBufferDynamic;
        break;

    case ShaderBuffer::Type::StructuredDynamic:
        type = vk::DescriptorType::eStorageBufferDynamic;
        break;
    }

    if (layout->getInfo().bindings[binding].type != type) {
        Logger::ERROR("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout.getIdentifier());
        return false;
    }

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    const std::vector<Buffer> &buffers = storage->getBuffers();

    std::array<vk::DescriptorBufferInfo, detail::MAX_FRAMES_IN_FLIGHT> bufferInfos;
    std::array<vk::WriteDescriptorSet, detail::MAX_FRAMES_IN_FLIGHT> writes;

    for (std::uint32_t frame = 0; frame < detail::MAX_FRAMES_IN_FLIGHT; frame++) {
        bufferInfos[frame] = vk::DescriptorBufferInfo{
            .buffer = buffers[frame].getData().buffer,
            .offset = layout->getInfo().bindings[binding].offset,
            .range = layout->getInfo().bindings[binding].range,
        };

        writes[frame] = {
            .dstSet = m_AllocatedLayouts[layout].sets[frame],
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = &bufferInfos[frame],
        };
    }

    App::Device.updateDescriptorSets(writes, {});

    return true;
}

bool PipelineLayoutAllocator::attach(const Resource<PipelineLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding) {
    if (layout->getInfo().bindings[binding].type != vk::DescriptorType::eCombinedImageSampler) {
        Logger::ERROR("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout.getIdentifier());
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

    std::array<vk::WriteDescriptorSet, detail::MAX_FRAMES_IN_FLIGHT> writes;
    for (std::uint32_t frame = 0; frame < detail::MAX_FRAMES_IN_FLIGHT; frame++) {
        writes[frame] = {
            .dstSet = m_AllocatedLayouts[layout].sets[frame],
            .dstBinding = static_cast<std::uint32_t>(binding),
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &imageInfo,
        };
    }

    App::Device.updateDescriptorSets(writes, {});
    return true;
}

bool PipelineLayoutAllocator::attach(const RenderContext &context, const Resource<PipelineLayout> &layout, const Resource<ShaderBuffer> &storage, std::uint32_t binding) {
    vk::DescriptorType type;

    switch (storage->getType()) {
    case ShaderBuffer::Type::Constant:
        type = vk::DescriptorType::eUniformBuffer;
        break;

    case ShaderBuffer::Type::Structured:
        type = vk::DescriptorType::eStorageBuffer;
        break;

    case ShaderBuffer::Type::ConstantDynamic:
        type = vk::DescriptorType::eUniformBufferDynamic;
        break;

    case ShaderBuffer::Type::StructuredDynamic:
        type = vk::DescriptorType::eStorageBufferDynamic;
        break;
    }

    if (layout->getInfo().bindings[binding].type != type) {
        Logger::ERROR("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout.getIdentifier());
        return false;
    }

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    const std::vector<Buffer> &buffers = storage->getBuffers();

    vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo{
        .buffer = buffers[context.frameInFlight].getData().buffer,
        .offset = layout->getInfo().bindings[binding].offset,
        .range = layout->getInfo().bindings[binding].range,
    };

    vk::WriteDescriptorSet write = {
        .dstSet = m_AllocatedLayouts[layout].sets[context.frameInFlight],
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &bufferInfo,
    };

    App::Device.updateDescriptorSets(write, {});

    return true;
}

bool PipelineLayoutAllocator::attach(const RenderContext &context, const Resource<PipelineLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding) {
    if (layout->getInfo().bindings[binding].type != vk::DescriptorType::eCombinedImageSampler) {
        Logger::ERROR("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout.getIdentifier());
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

    vk::WriteDescriptorSet write = {
        .dstSet = m_AllocatedLayouts[layout].sets[context.frameInFlight],
        .dstBinding = static_cast<std::uint32_t>(binding),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfo,
    };

    App::Device.updateDescriptorSets(write, {});
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

void PipelineLayoutAllocator::bind(const RenderContext &context, const Resource<RenderPipeline> &pipeline, std::uint32_t idx) {
    for (std::size_t i = 0; i < pipeline->getInfo().layouts.size(); i++) {
        const Resource<PipelineLayout> &layout = pipeline->getInfo().layouts[i];

        if (!m_AllocatedLayouts.contains(layout)) {
            Logger::DEBUG("[PipelineLayoutAllocator] Allocating layout '{}' for pipeline '{}'", layout.getIdentifier(), pipeline.getIdentifier());
            if (!allocate(layout)) {
                Logger::CRITICAL("[PipelineLayoutAllocator] Could not allocate layout to bind PipelineLayout.");
                continue;
            }
        }

        std::vector<std::uint32_t> dynamicOffsets;
        for (const auto binding : layout->getInfo().bindings) {
            std::uint32_t minDynamicOffset = 0;

            if (binding.type == vk::DescriptorType::eStorageBufferDynamic) {
                minDynamicOffset = App::PhysicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
            } else if (binding.type == vk::DescriptorType::eUniformBufferDynamic) {
                minDynamicOffset = App::PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
            } else {
                continue;
            }

            // offsets must be multiples of driver defined minimums
            dynamicOffsets.emplace_back(((binding.range - 1) / minDynamicOffset + 1) * minDynamicOffset * idx);
        }

        context.command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->m_Layout, i, m_AllocatedLayouts[layout].sets[context.frameInFlight], dynamicOffsets);
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
