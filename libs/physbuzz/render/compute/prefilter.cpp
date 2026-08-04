#include "prefilter.hpp"

#include "../../app/application.hpp"
#include "../../graphics/layout.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/pipeline.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool PipelinePrefilter::build() {
    if (ResourceRegistry<ComputePipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<DescriptorLayout>::contains(ResourceLayout)) {
        success &= ResourceRegistry<DescriptorLayout>::insert(
            ResourceLayout,
            {{
                .bindings = {
                    {
                        // environment map
                        .type = DescriptorLayout::Type::eCombinedImageSampler,
                        .stage = DescriptorLayout::ShaderStageFlags::eCompute,
                    },
                    {
                        // prefilter map
                        .type = DescriptorLayout::Type::eStorageImage,
                        .flags = DescriptorLayout::DescriptorBindingFlags::ePartiallyBound,
                        .stage = DescriptorLayout::ShaderStageFlags::eCompute,
                        .count = 15, // assumed from maxImageDimension2D = 16384 offline TODO get this value from VkPhysicalDevicegetImageFormatProperties
                    },
                },
                .lifetime = DescriptorLayout::Lifetime::Global,
            }});
    }

    success &= ResourceRegistry<ComputePipeline>::insert(
        Resource,
        {
            {
                .module = "builtin/compute/prefilter",
            },
            {
                .layouts = {
                    .resources = {
                        ResourceLayout,
                    },
                    .pushConstantRanges = {
                        {
                            .stageFlags = GraphicsPipeline::PushConstantsStageFlags::eAll,
                            .size = sizeof(PushConstants),
                        },
                    },
                },
            },
        });

    return success;
}

} // namespace Builtin

PrefilterCompute::PrefilterCompute(const Info &info)
    : m_Info(info) {}

bool PrefilterCompute::build() {
    // build pipeline
    if (!Builtin::PipelinePrefilter::build()) {
        Logger::ERROR("[PrefilterCompute] Could not build prefilter pipeline.");
        return false;
    }

    bool success = true;

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

    success &= ResourceRegistry<Texture>::insert(
        m_Resource,
        {{
            .type = Texture::Type::Dim2D,
            .usage = Texture::Usage::Storage,
            .mipLevels = mipLevels,
            .sampler = {{Sampler::Type::Linear}},
            .additionalViews = additionalViews,
        }},
        resolution);

    m_Graph.add(
        Output,
        {
            .description = {
                .textures = {
                    .input = {{
                        m_Info.environmentMap,
                        {
                            .stage = RenderNode::Stage::Compute,
                            .subresourceRanges = subresourceRanges,
                        },
                    }},
                    .output = {{
                        m_Resource,
                        {
                            .stage = RenderNode::Stage::Compute,
                            .subresourceRanges = subresourceRanges,
                        },
                    }},
                },
            },
            .execute = [this, mipLevels](Scene *scene, const RenderContext &context) {
                ZoneScopedN("PrefilterCompute/Execute");
                TracyVkZone(context.tracy, context.command, "PrefilterCompute");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                Builtin::PipelinePrefilter::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelinePrefilter::Resource);

                glm::ivec2 mipResolution = m_Info.environmentMap->getSize();

                for (std::uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
                    Builtin::PipelinePrefilter::PushConstants pushConstants = {
                        .mipLevel = mipLevel,
                        .roughness = static_cast<float>(mipLevel) / static_cast<float>(mipLevels - 1),
                    };

                    Builtin::PipelinePrefilter::Resource->updatePushConstants(context, GraphicsPipeline::PushConstantsStageFlags::eAll, std::as_bytes(std::span(&pushConstants, 1)), 0);

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
        success &= App::LayoutAllocator.write(
            Builtin::PipelinePrefilter::ResourceLayout,
            m_Info.environmentMap,
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = Image::RemainingMipLevels,
                    .layerCount = 1,
                },
            },
            0);

        // write each mip level view into the descriptor array
        for (std::uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
            success &= App::LayoutAllocator.write(
                Builtin::PipelinePrefilter::ResourceLayout,
                m_Resource,
                Image::ViewInfo{
                    .type = Image::ViewType::e2D,
                    .subresourceRange = {
                        .aspectMask = Image::AspectFlags::eColor,
                        .baseMipLevel = mipLevel,
                        .levelCount = 1,
                        .layerCount = 1,
                    },
                },
                1, mipLevel);
        }
    }

    return success;
}

bool PrefilterCompute::destroy() {
    m_Resource->destroy();

    return true;
}

const RenderGraph &PrefilterCompute::getGraph() const {
    return m_Graph;
}

const Resource<Texture> &PrefilterCompute::getPrefilterMap() const {
    return m_Resource;
}

const PrefilterCompute::Info &PrefilterCompute::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
