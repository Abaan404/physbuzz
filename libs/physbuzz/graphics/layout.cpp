#include "layout.hpp"

#include "../app/application.hpp"
#include "../debug/macros.hpp"
#include "../events/descriptor.hpp"
#include "descriptors/attachment.hpp"
#include "descriptors/dynamic.hpp"
#include "descriptors/sampler.hpp"
#include "descriptors/texture.hpp"
#include "pipeline.hpp"

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
    bindingFlags.reserve(m_Info.bindings.size());

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

    std::unordered_map<Resource<PipelineLayout>, Allocation> allocatedLayouts = m_AllocatedLayouts;

    for (auto [layout, _] : allocatedLayouts) {
        deallocate(layout);
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

bool PipelineLayoutAllocator::write(const Resource<PipelineLayout> &layout, const Resource<DynamicBuffer> &buffer, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type;

    switch (buffer->getInfo().type) {
    case DynamicBuffer::Type::Constant:
        type = vk::DescriptorType::eUniformBuffer;
        break;

    case DynamicBuffer::Type::Structured:
        type = vk::DescriptorType::eStorageBuffer;
        break;

    case DynamicBuffer::Type::ConstantDynamic:
        type = vk::DescriptorType::eUniformBufferDynamic;
        break;

    case DynamicBuffer::Type::StructuredDynamic:
        type = vk::DescriptorType::eStorageBufferDynamic;
        break;
    }

    PBZ_ASSERT(
        binding < layout->getInfo().bindings.size() && layout->getInfo().bindings[binding].type == type,
        std::format("[PipelineLayoutAllocator] Invalid type at binding {} element {} for dynamic buffer '{}'", binding, element, layout));
    PBZ_ASSERT(
        layout->getInfo().lifetime == PipelineLayout::Lifetime::PerFrame,
        std::format("[PipelineLayoutAllocator] Incompatible lifetime for dynamic buffer '{}' and layout '{}'", buffer, layout));

    // check if binding is cached
    auto &[_, entry] = *m_WrittenBuffers.insert({buffer, {}}).first;

    if (entry.layouts.contains(layout)) {
        buffer->eraseCallback<OnDynamicBufferRealloc>(entry.layouts.at(layout).realloc);
    }

    entry.layouts.insert({
        layout,
        {
            .realloc = buffer->addCallback<OnDynamicBufferRealloc>([this, layout, buffer](const OnDynamicBufferRealloc &event) {
                WriteInfo &writeInfo = m_WrittenBuffers.at(buffer).layouts.at(layout);
                rewrite(layout, buffer, event.context, writeInfo.binding, writeInfo.element);
            }),
            .binding = binding,
            .element = element,
        },
    });

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    const std::array<DynamicBuffer::Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = buffer->getRingData();
    std::array<vk::DescriptorBufferInfo, detail::MAX_FRAMES_IN_FLIGHT> bufferInfos;
    std::array<vk::WriteDescriptorSet, detail::MAX_FRAMES_IN_FLIGHT> writes;

    for (std::uint32_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        bufferInfos[i] = vk::DescriptorBufferInfo{
            .buffer = ringData[i].buffer.getData().buffer,
            .offset = layout->getInfo().bindings[binding].offset + element * layout->getInfo().bindings[binding].range,
            .range = layout->getInfo().bindings[binding].range,
        };

        writes[i] = {
            .dstSet = m_AllocatedLayouts[layout].sets[i],
            .dstBinding = binding,
            .dstArrayElement = element,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = &bufferInfos[i],
        };
    }

    App::Device.updateDescriptorSets(writes, {});

    return true;
}

bool PipelineLayoutAllocator::write(const Resource<PipelineLayout> &layout, const Resource<Texture> &texture, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type;

    switch (texture->getInfo().type) {
    case Texture::Type::Dim2D:
    case Texture::Type::Cube:
        if (texture->getInfo().sampler.type == Sampler::Type::None) {
            type = vk::DescriptorType::eSampledImage;
        } else {
            type = vk::DescriptorType::eCombinedImageSampler;
        }

        break;
    }

    PBZ_ASSERT(
        binding < layout->getInfo().bindings.size() && layout->getInfo().bindings[binding].type == type,
        std::format("[PipelineLayoutAllocator] Invalid type at binding {} element {} for texture '{}'", binding, element, layout));
    PBZ_ASSERT(
        layout->getInfo().lifetime == PipelineLayout::Lifetime::Global,
        std::format("[PipelineLayoutAllocator] Incompatible lifetime for texture '{}' and layout '{}'", texture, layout));

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    // check if binding is cached
    auto &[_, entry] = *m_WrittenTextures.insert({texture, {}}).first;

    if (entry.layouts.contains(layout)) {
        texture->eraseCallback<OnTextureRealloc>(entry.layouts.at(layout).realloc);
    }

    entry.layouts.insert({
        layout,
        {
            .realloc = texture->addCallback<OnTextureRealloc>([this, layout, texture](const OnTextureRealloc &event) {
                WriteInfo &writeInfo = m_WrittenTextures.at(texture).layouts.at(layout);
                rewrite(layout, texture, event.context, writeInfo.binding, writeInfo.element);
            }),
            .binding = binding,
            .element = element,
        },
    });

    vk::DescriptorImageInfo imageInfo = {
        .sampler = texture->getData().sampler.getData().sampler,
        .imageView = texture->getData().view,
        .imageLayout = texture->getData().layout,
    };

    vk::WriteDescriptorSet writes = {
        .dstSet = m_AllocatedLayouts[layout].sets[0],
        .dstBinding = static_cast<std::uint32_t>(binding),
        .dstArrayElement = element,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &imageInfo,
    };

    App::Device.updateDescriptorSets(writes, {});

    return true;
}

bool PipelineLayoutAllocator::write(const Resource<PipelineLayout> &layout, const Resource<Attachment> &attachment, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type = vk::DescriptorType::eInputAttachment;

    PBZ_ASSERT(
        binding < layout->getInfo().bindings.size() && layout->getInfo().bindings[binding].type == type,
        std::format("[PipelineLayoutAllocator] Invalid type at binding {} element {} for attachment '{}'", binding, element, layout));
    PBZ_ASSERT(
        layout->getInfo().lifetime == PipelineLayout::Lifetime::PerFrame,
        std::format("[PipelineLayoutAllocator] Incompatible lifetime for attachment '{}' and layout '{}'", attachment, layout));

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    // check if binding is cached
    auto &[_, entry] = *m_WrittenAttachments.insert({attachment, {}}).first;

    if (entry.layouts.contains(layout)) {
        attachment->eraseCallback<OnAttachmentRealloc>(entry.layouts.at(layout).realloc);
    }

    entry.layouts.insert({
        layout,
        {
            .realloc = attachment->addCallback<OnAttachmentRealloc>([this, layout, attachment](const OnAttachmentRealloc &event) {
                WriteInfo &writeInfo = m_WrittenAttachments.at(attachment).layouts.at(layout);
                rewrite(layout, attachment, event.context, writeInfo.binding, writeInfo.element);
            }),
            .binding = binding,
            .element = element,
        },
    });

    const std::array<Attachment::Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = attachment->getRingData();
    std::array<vk::DescriptorImageInfo, detail::MAX_FRAMES_IN_FLIGHT> imageInfos;
    std::array<vk::WriteDescriptorSet, detail::MAX_FRAMES_IN_FLIGHT> writes;

    for (std::uint32_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
        imageInfos[i] = vk::DescriptorImageInfo{
            .imageView = ringData[i].view,
            .imageLayout = ringData[i].layout,
        };

        writes[i] = {
            .dstSet = m_AllocatedLayouts[layout].sets[i],
            .dstBinding = static_cast<std::uint32_t>(binding),
            .dstArrayElement = element,
            .descriptorCount = 1,
            .descriptorType = type,
            .pImageInfo = &imageInfos[i],
        };
    }

    App::Device.updateDescriptorSets(writes, {});

    return true;
}

bool PipelineLayoutAllocator::write(const Resource<PipelineLayout> &layout, const Resource<Sampler> &sampler, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type = vk::DescriptorType::eSampler;

    PBZ_ASSERT(
        binding < layout->getInfo().bindings.size() && layout->getInfo().bindings[binding].type == type,
        std::format("[PipelineLayoutAllocator] Invalid type at binding {} element {} for sampler '{}'", binding, element, layout));
    PBZ_ASSERT(
        layout->getInfo().lifetime == PipelineLayout::Lifetime::Global,
        std::format("[PipelineLayoutAllocator] Incompatible lifetime for sampler '{}' and layout '{}'", sampler, layout));

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    vk::DescriptorImageInfo imageInfo = {
        .sampler = sampler->getData().sampler,
    };

    vk::WriteDescriptorSet writes = {
        .dstSet = m_AllocatedLayouts[layout].sets[0],
        .dstBinding = static_cast<std::uint32_t>(binding),
        .dstArrayElement = element,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &imageInfo,
    };

    App::Device.updateDescriptorSets(writes, {});

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

void PipelineLayoutAllocator::bind(const RenderContext &context, const RenderPipeline &pipeline, std::uint32_t idx) {
    for (std::size_t i = 0; i < pipeline.getInfo().layouts.resources.size(); i++) {
        const Resource<PipelineLayout> &layout = pipeline.getInfo().layouts.resources[i];
        PBZ_ASSERT(m_AllocatedLayouts.contains(layout), std::format("[PipelineLayoutAllocator] Unallocated layout '{}'", layout));

        std::vector<std::uint32_t> dynamicOffsets;
        dynamicOffsets.reserve(layout->getInfo().bindings.size()); // reserve extra even if it wont be all used

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
        case PipelineLayout::Lifetime::Global:
            index = 0;
            break;
        case PipelineLayout::Lifetime::PerFrame:
            index = context.frameInFlight;
            break;
        }

        std::lock_guard<std::mutex> lock(pipeline.m_ReloadMutex);
        context.command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.m_Layout, i, m_AllocatedLayouts[layout].sets[index], dynamicOffsets);
    }
}

bool PipelineLayoutAllocator::rewrite(const Resource<PipelineLayout> &layout, const Resource<DynamicBuffer> &buffer, const RenderContext &context, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type;

    switch (buffer->getInfo().type) {
    case DynamicBuffer::Type::Constant:
        type = vk::DescriptorType::eUniformBuffer;
        break;

    case DynamicBuffer::Type::Structured:
        type = vk::DescriptorType::eStorageBuffer;
        break;

    case DynamicBuffer::Type::ConstantDynamic:
        type = vk::DescriptorType::eUniformBufferDynamic;
        break;

    case DynamicBuffer::Type::StructuredDynamic:
        type = vk::DescriptorType::eStorageBufferDynamic;
        break;
    }

    PBZ_ASSERT(
        binding < layout->getInfo().bindings.size() && layout->getInfo().bindings[binding].type == type,
        std::format("[PipelineLayoutAllocator] Invalid type at binding {} element {} for dynamic buffer '{}'", binding, element, layout));
    PBZ_ASSERT(
        layout->getInfo().lifetime == PipelineLayout::Lifetime::PerFrame,
        std::format("[PipelineLayoutAllocator] Incompatible lifetime for dynamic buffer at binding '{}' and layout '{}'", binding, layout));

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    const std::array<DynamicBuffer::Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = buffer->getRingData();

    vk::DescriptorBufferInfo bufferInfo = {
        .buffer = ringData[context.frameInFlight].buffer.getData().buffer,
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

bool PipelineLayoutAllocator::rewrite(const Resource<PipelineLayout> &layout, const Resource<Texture> &texture, const RenderContext &context, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type;

    switch (texture->getInfo().type) {
    case Texture::Type::Dim2D:
    case Texture::Type::Cube:
        if (texture->getInfo().sampler.type == Sampler::Type::None) {
            type = vk::DescriptorType::eSampledImage;
        } else {
            type = vk::DescriptorType::eCombinedImageSampler;
        }

        break;
    }

    PBZ_ASSERT(
        binding < layout->getInfo().bindings.size() && layout->getInfo().bindings[binding].type == type,
        std::format("[PipelineLayoutAllocator] Invalid type at binding {} element {} for texture '{}'", binding, element, layout));
    PBZ_ASSERT(
        layout->getInfo().lifetime == PipelineLayout::Lifetime::Global,
        std::format("[PipelineLayoutAllocator] Incompatible lifetime for texture at binding '{}' and layout '{}'", binding, layout));

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    vk::DescriptorImageInfo imageInfo = {
        .sampler = texture->getData().sampler.getData().sampler,
        .imageView = texture->getData().view,
        .imageLayout = texture->getData().layout,
    };

    vk::WriteDescriptorSet writes = {
        .dstSet = m_AllocatedLayouts[layout].sets[0],
        .dstBinding = static_cast<std::uint32_t>(binding),
        .dstArrayElement = element,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &imageInfo,
    };

    App::Device.updateDescriptorSets(writes, {});
    return true;
}

bool PipelineLayoutAllocator::rewrite(const Resource<PipelineLayout> &layout, const Resource<Attachment> &attachment, const RenderContext &context, std::uint32_t binding, std::uint32_t element) {
    vk::DescriptorType type = vk::DescriptorType::eInputAttachment;

    PBZ_ASSERT(
        binding < layout->getInfo().bindings.size() && layout->getInfo().bindings[binding].type == type,
        std::format("[PipelineLayoutAllocator] Invalid type at binding {} element {} for attachment '{}'", binding, element, layout));
    PBZ_ASSERT(
        layout->getInfo().lifetime == PipelineLayout::Lifetime::PerFrame,
        std::format("[PipelineLayoutAllocator] Incompatible lifetime for attachment at binding '{}' and layout '{}'", binding, layout));

    if (!m_AllocatedLayouts.contains(layout)) {
        allocate(layout);
    }

    const std::array<Attachment::Data, detail::MAX_FRAMES_IN_FLIGHT> &ringData = attachment->getRingData();

    vk::DescriptorImageInfo imageInfo = {
        .imageView = ringData[context.frameInFlight].view,
        .imageLayout = ringData[context.frameInFlight].layout,
    };

    vk::WriteDescriptorSet write = {
        .dstSet = m_AllocatedLayouts[layout].sets[context.frameInFlight],
        .dstBinding = binding,
        .dstArrayElement = element,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &imageInfo,
    };

    App::Device.updateDescriptorSets(write, {});

    return true;
}

bool PipelineLayoutAllocator::allocate(const Resource<PipelineLayout> &layout) {
    if (m_AllocatedLayouts.contains(layout)) {
        Logger::WARNING("[PipelineLayoutAllocator] Trying to allocate an allocated layout.");
        return true;
    }

    std::uint32_t count;
    switch (layout->getInfo().lifetime) {
    case PipelineLayout::Lifetime::Global:
        count = 1;
        break;
    case PipelineLayout::Lifetime::PerFrame:
        count = detail::MAX_FRAMES_IN_FLIGHT;
        break;
    }

    std::vector<vk::DescriptorSetLayout> setLayouts;
    setLayouts.reserve(count);

    for (std::size_t i = 0; i < count; i++) {
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

    for (auto &[resource, entry] : m_WrittenBuffers) {
        for (const auto &[layout, writeInfo] : entry.layouts) {
            resource->eraseCallback<OnDynamicBufferRealloc>(writeInfo.realloc);
        }

        entry.layouts.erase(layout);
    }

    for (auto &[resource, entry] : m_WrittenTextures) {
        for (const auto &[layout, writeInfo] : entry.layouts) {
            resource->eraseCallback<OnTextureRealloc>(writeInfo.realloc);
        }

        entry.layouts.erase(layout);
    }

    for (auto &[resource, entry] : m_WrittenAttachments) {
        for (const auto &[layout, writeInfo] : entry.layouts) {
            resource->eraseCallback<OnAttachmentRealloc>(writeInfo.realloc);
        }

        entry.layouts.erase(layout);
    }

    Allocation &alloc = m_AllocatedLayouts[layout];
    PBZ_VK_CHECK_RESULT(App::Device.freeDescriptorSets(alloc.allocatorPool, alloc.sets));

    m_AllocatedLayouts.erase(layout);
    return true;
}

vk::DescriptorPool PipelineLayoutAllocator::createPool() {
    std::vector<vk::DescriptorPoolSize> sizes;
    sizes.reserve(m_Info.poolSizes.size());

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
