#include "layout.hpp"

#include "../app/application.hpp"
#include "../debug/macros.hpp"
#include "layouts/defines.hpp"
#include "layouts/shaderbuffer.hpp"
#include "layouts/texture.hpp"
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
    std::vector<vk::DescriptorBindingFlags> bindingFlags;
    layoutBindings.reserve(m_Info.bindings.size());

    for (std::uint32_t i = 0; i < m_Info.bindings.size(); i++) {
        layoutBindings.emplace_back<vk::DescriptorSetLayoutBinding>({
            .binding = i,
            .descriptorType = m_Info.bindings[i].type,
            .descriptorCount = m_Info.bindings[i].count,
            .stageFlags = m_Info.bindings[i].stage,
        });

        bindingFlags.emplace_back(m_Info.bindings[i].flags);
    }

    vk::StructureChain<vk::DescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo> chain = {
        {
            .flags = m_Info.flags,
            .bindingCount = static_cast<std::uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        },
        {
            .bindingCount = static_cast<std::uint32_t>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data(),
        },
    };

    m_Layout = PBZ_VK_CHECK(App::Device.createDescriptorSetLayout(chain.get()));

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
        PBZ_VK_CHECK_RESULT(App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets));
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

bool PipelineLayoutAllocator::attach(const Resource<PipelineLayout> &layout, const Resource<ShaderBuffer> &storage, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type;

    switch (storage->getInfo().type) {
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

    PBZ_ASSERT(layout->getInfo().bindings[binding].type == type, std::format("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout));
    PBZ_ASSERT(layout->getInfo().lifetime == storage->getInfo().lifetime, std::format("[PipelineLayoutAllocator] Incompatible lifetime for shaderbuffer '{}' and layout '{}'", storage, layout));

    const std::vector<Buffer> &buffers = storage->getBuffers();

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::WriteDescriptorSet> writes;

    bufferInfos.reserve(buffers.size());
    writes.reserve(buffers.size());

    for (std::uint32_t i = 0; i < buffers.size(); i++) {
        bufferInfos.emplace_back<vk::DescriptorBufferInfo>({
            .buffer = buffers[i].getData().buffer,
            .offset = layout->getInfo().bindings[binding].offset + element * layout->getInfo().bindings[binding].range,
            .range = layout->getInfo().bindings[binding].range,
        });

        writes.emplace_back<vk::WriteDescriptorSet>({
            .dstSet = m_AllocatedLayouts[layout].sets[i],
            .dstBinding = binding,
            .dstArrayElement = element,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = &bufferInfos[i],
        });
    }

    App::Device.updateDescriptorSets(writes, {});

    return true;
}

bool PipelineLayoutAllocator::attach(const Resource<PipelineLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding, std::uint32_t element) {
    if (layout->getInfo().bindings[binding].type != vk::DescriptorType::eCombinedImageSampler) {
        Logger::ERROR("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout);
        return false;
    }

    PBZ_ASSERT(layout->getInfo().bindings[binding].type == vk::DescriptorType::eCombinedImageSampler, std::format("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout));
    PBZ_ASSERT(layout->getInfo().lifetime == LayoutLifetime::Global, std::format("[PipelineLayoutAllocator] Incompatible lifetime for shaderbuffer '{}' and layout '{}'", texture, layout));

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    vk::DescriptorImageInfo imageInfo = {
        .sampler = texture->getSampler(),
        .imageView = texture->getImageView(),
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
    };

    std::vector<vk::WriteDescriptorSet> writes;
    for (std::uint32_t i = 0; i < 1; i++) {
        writes.emplace_back<vk::WriteDescriptorSet>({
            .dstSet = m_AllocatedLayouts[layout].sets[i],
            .dstBinding = static_cast<std::uint32_t>(binding),
            .dstArrayElement = element,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &imageInfo,
        });
    }

    App::Device.updateDescriptorSets(writes, {});
    return true;
}

bool PipelineLayoutAllocator::reattach(const RenderContext &context, const Resource<PipelineLayout> &layout, const Resource<ShaderBuffer> &storage, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type;

    switch (storage->getInfo().type) {
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

    PBZ_ASSERT(layout->getInfo().lifetime == LayoutLifetime::PerFrame, std::format("[PipelineLayoutAllocator] Incompatible layout when reattaching layout '{}'", binding, layout));
    PBZ_ASSERT(storage->getInfo().lifetime == LayoutLifetime::PerFrame, std::format("[PipelineLayoutAllocator] Incompatible shader buffer when reattaching layout '{}'", binding, storage));
    PBZ_ASSERT(layout->getInfo().bindings[binding].type == type, std::format("[PipelineLayoutAllocator] Invalid type at binding {} for resource '{}'", binding, layout));

    const std::vector<Buffer> &buffers = storage->getBuffers();

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo{
        .buffer = buffers[context.frameInFlight].getData().buffer,
        .offset = layout->getInfo().bindings[binding].offset + element * layout->getInfo().bindings[binding].range,
        .range = layout->getInfo().bindings[binding].range,
    };

    vk::WriteDescriptorSet write = {
        .dstSet = m_AllocatedLayouts[layout].sets[context.frameInFlight],
        .dstBinding = binding,
        .dstArrayElement = element,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &bufferInfo,
    };

    App::Device.updateDescriptorSets(write, {});

    return true;
}

void PipelineLayoutAllocator::reset() {
    // free all allocated sets, not necessary since we're resetting the pool, however since we're no longer tracking them we might aswell.
    for (const auto &[layout, alloc] : m_AllocatedLayouts) {
        PBZ_VK_CHECK_RESULT(App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets));
    }

    for (const auto &pool : m_UsedPools) {
        PBZ_VK_CHECK_RESULT(App::Device.resetDescriptorPool(pool));
    }

    for (const auto &pool : m_FreePools) {
        PBZ_VK_CHECK_RESULT(App::Device.resetDescriptorPool(pool));
    }

    m_FreePools.insert(m_FreePools.end(), std::make_move_iterator(m_UsedPools.begin()), std::make_move_iterator(m_UsedPools.end()));
    m_UsedPools.clear();
    m_AllocatedLayouts.clear();
}

void PipelineLayoutAllocator::bind(const RenderContext &context, const Resource<RenderPipeline> &pipeline, std::uint32_t idx) {
    for (std::size_t i = 0; i < pipeline->getInfo().layouts.resources.size(); i++) {
        const Resource<PipelineLayout> &layout = pipeline->getInfo().layouts.resources[i];
        PBZ_ASSERT(m_AllocatedLayouts.contains(layout), std::format("[PipelineLayoutAllocator] Unallocated layout '{}'", layout));

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

        std::uint32_t index;
        switch (layout->getInfo().lifetime) {
        case LayoutLifetime::Global:
            index = 0;
            break;
        case LayoutLifetime::PerFrame:
            index = context.frameInFlight;
            break;
        }

        context.command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->m_Layout, i, m_AllocatedLayouts[layout].sets[index], dynamicOffsets);
    }
}

bool PipelineLayoutAllocator::allocate(const Resource<PipelineLayout> &layout) {
    if (m_AllocatedLayouts.contains(layout)) {
        Logger::WARNING("[PipelineLayoutAllocator] Trying to allocate an allocated layout.");
        return true;
    }

    std::vector<vk::DescriptorSetLayout> setLayouts;
    setLayouts.reserve(detail::getLayoutLifetimeSetCount(layout->getInfo().lifetime));

    for (std::size_t i = 0; i < detail::getLayoutLifetimeSetCount(layout->getInfo().lifetime); i++) {
        setLayouts.emplace_back(layout->m_Layout);
    }

    vk::DescriptorSetAllocateInfo allocateInfo = {
        .descriptorPool = m_CurrentPool,
        .descriptorSetCount = static_cast<std::uint32_t>(setLayouts.size()),
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
    PBZ_VK_CHECK_RESULT(App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets));

    m_AllocatedLayouts.erase(layout);
    return true;
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
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
        .maxSets = m_Info.chunkSize * detail::MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = static_cast<std::uint32_t>(sizes.size()),
        .pPoolSizes = sizes.data(),
    }));
}

} // namespace Physbuzz
