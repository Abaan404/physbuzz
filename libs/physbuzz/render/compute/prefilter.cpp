#include "prefilter.hpp"

#include "../../app/application.hpp"
#include "../../graphics/layout.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/pipeline.hpp"
#include <format>
#include <tracy/Tracy.hpp>

namespace Physbuzz {

PrefilterCompute::PrefilterCompute(const Info &info)
    : m_Info(info) {}

bool PrefilterCompute::build() {
    std::uint32_t maxMipLevels = m_Info.environmentMap->getData().image.getImageFormatProperties().maxMipLevels;

    if (!buildResources(maxMipLevels)) {
        Logger::ERROR("[PrefilterCompute] Could not build prefilter pipeline.");
        return false;
    }

    bool success = true;

    Resources::Specialization specialization = {
        .isCubemap = m_Info.environmentMap->getInfo().type == Texture::Type::Cube,
    };

    success &= m_Resources.pipeline->specialize(specialization);

    std::vector<Image::SubresourceRange> subresourceRanges;
    std::vector<Image::ViewInfo> additionalViews;

    glm::uvec3 resolution = m_Info.environmentMap->getSize();
    const std::uint32_t mipLevels = std::floor(std::log2(std::max(resolution.x, resolution.y))) + 1;

    // since each mip level will be rendered to, the prefilter need a view into each of its mip
    for (std::uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
        const Image::SubresourceRange &subresourceRange = subresourceRanges.emplace_back<Image::SubresourceRange>({
            .aspectMask = Image::AspectFlags::eColor,
            .baseMipLevel = mipLevel,
            .levelCount = 1,
            .layerCount = 1,
        });

        additionalViews.emplace_back<Image::ViewInfo>({
            .type = Image::ViewType::e2D,
            .subresourceRange = subresourceRange,
        });
    }

    auto resource = ResourceRegistry<Texture>::insert(
        std::format("{}/prefilter", m_Info.environmentMap.getName()),
        {{
            .type = Texture::Type::Dim2D,
            .usage = Texture::Usage::Storage,
            .mipLevels = mipLevels,
            .sampler = {{Sampler::Type::Linear}},
            .additionalViews = additionalViews,
        }},
        resolution);

    if (!resource) {
        return false;
    }

    m_PrefilterMap = std::move(*resource);

    m_Graph.add(
        Output,
        {
            .description = {
                .textures = {
                    .input = {{
                        m_Info.environmentMap,
                        {
                            .stage = RenderNode::Stage::Compute,
                            .subresourceRanges = {{
                                .aspectMask = Image::AspectFlags::eColor,
                                .levelCount = 1,
                                .layerCount = 1,
                            }},
                        },
                    }},
                    .output = {{
                        m_PrefilterMap,
                        {
                            .stage = RenderNode::Stage::Compute,
                            .subresourceRanges = subresourceRanges,
                        },
                    }},
                },
            },
            .execute = [this, mipLevels](const RenderContext &context) {
                ZoneScopedN("PrefilterCompute/Execute");
                TracyVkZone(context.tracy, context.command, "PrefilterCompute");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                m_Resources.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.pipeline);

                glm::ivec2 mipResolution = m_Info.environmentMap->getSize();

                for (std::uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
                    Resources::PushConstants pushConstants = {
                        .mipLevel = mipLevel,
                        .roughness = static_cast<float>(mipLevel) / static_cast<float>(mipLevels - 1),
                    };

                    m_Resources.pipeline->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

                    // to constexpr the ceil (TODO cpp23 makes ceil constexpr)
                    std::uint32_t groupSize = 8;
                    glm::uvec2 groupCount = {
                        (mipResolution.x + groupSize - 1) / groupSize,
                        (mipResolution.y + groupSize - 1) / groupSize,
                    };

                    context.command.dispatch(groupCount.x, groupCount.y, 1);

                    mipResolution = glm::max(glm::ivec2{1}, mipResolution / 2);
                }
            },
        });

    success &= m_Graph.compile();

    if (success) {
        if (specialization.isCubemap) {
            success &= App::LayoutAllocator.write(
                m_Resources.layout,
                m_Info.environmentMap,
                {
                    .type = Image::ViewType::eCube,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .levelCount = Image::RemainingMipLevels,
                        .layerCount = 6,
                    },
                },
                0);
        } else {
            success &= App::LayoutAllocator.write(
                m_Resources.layout,
                m_Info.environmentMap,
                {
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .levelCount = Image::RemainingMipLevels,
                        .layerCount = 1,
                    },
                },
                1);
        }

        // write each mip level view into the descriptor array
        for (std::uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
            success &= App::LayoutAllocator.write(
                m_Resources.layout,
                m_PrefilterMap,
                Image::ViewInfo{
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .baseMipLevel = mipLevel,
                        .levelCount = 1,
                        .layerCount = 1,
                    },
                },
                2, mipLevel);
        }
    }

    return success;
}

bool PrefilterCompute::destroy() {
    return true;
}

const RenderGraph &PrefilterCompute::getGraph() const {
    return m_Graph;
}

const Resource<Texture> &PrefilterCompute::getPrefilterMap() const {
    return m_PrefilterMap;
}

const PrefilterCompute::Info &PrefilterCompute::getInfo() const {
    return m_Info;
}

bool PrefilterCompute::buildResources(const std::uint32_t &maxMipLevels) {
    auto layout = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/prefilter",
        {{
            .bindings = {
                {
                    // environment map (cubemap)
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .flags = DescriptorLayout::DescriptorBindingFlags::ePartiallyBound,
                    .stage = DescriptorLayout::ShaderStageFlags::eCompute,
                },
                {
                    // environment map (equirectangular)
                    .type = DescriptorLayout::Type::eCombinedImageSampler,
                    .flags = DescriptorLayout::DescriptorBindingFlags::ePartiallyBound,
                    .stage = DescriptorLayout::ShaderStageFlags::eCompute,
                },
                {
                    // prefilter map
                    .type = DescriptorLayout::Type::eStorageImage,
                    .flags = DescriptorLayout::DescriptorBindingFlags::ePartiallyBound,
                    .stage = DescriptorLayout::ShaderStageFlags::eCompute,
                    .count = maxMipLevels,
                },
            },
            .lifetime = DescriptorLayout::Lifetime::Global,
        }});

    if (!layout) {
        return false;
    }

    auto pipeline = ResourceRegistry<ComputePipeline>::insert(
        "builtin/prefilter",
        {
            {
                .module = "builtin/compute/prefilter",
                .specialization = {
                    .offsets = {
                        offsetof(Resources::Specialization, isCubemap),
                    },
                    .size = sizeof(Resources::Specialization),
                },
            },
            {
                .layouts = {
                    .resources = {
                        *layout,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eAll,
                            .size = sizeof(Resources::PushConstants),
                        },
                    },
                },
            },
        });

    if (!pipeline) {
        return false;
    }

    m_Resources = {
        .layout = std::move(*layout),
        .pipeline = std::move(*pipeline),
    };

    return true;
}

} // namespace Physbuzz
