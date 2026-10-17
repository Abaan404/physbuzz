#include "irradiance.hpp"

#include "../../app/application.hpp"
#include "../../graphics/layout.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/pipeline.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

IrradianceCompute::IrradianceCompute(const Info &info)
    : m_Info(info) {}

bool IrradianceCompute::build() {
    if (!buildResources()) {
        Logger::ERROR("[IrradianceCompute] Could not build irradiance pipeline.");
        return false;
    }

    bool success = true;

    Resources::Specialization specialization = {
        .isCubemap = m_Info.environmentMap->getInfo().type == Texture::Type::Cube,
    };

    success &= m_Resources.pipeline->specialize(specialization);

    constexpr glm::vec3 resolution = {128, 64, 1};

    auto resource = ResourceRegistry<Texture>::insert(
        std::format("{}/irradiance", m_Info.environmentMap.getName()),
        {{
            .type = Texture::Type::Dim2D,
            .usage = Texture::Usage::Storage,
            .sampler = {{Sampler::Type::Linear}},
        }},
        resolution);

    if (!resource) {
        return false;
    }

    m_IrradianceMap = std::move(*resource);

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
                                .levelCount = Image::RemainingMipLevels,
                                .layerCount = 1,
                            }},
                        },
                    }},
                    .output = {{
                        m_IrradianceMap,
                        {
                            .stage = RenderNode::Stage::Compute,
                            .subresourceRanges = {{
                                .aspectMask = Image::AspectFlags::eColor,
                                .levelCount = Image::RemainingMipLevels,
                                .layerCount = 1,
                            }},
                        },
                    }},
                },
            },
            .execute = [this](const RenderContext &context) {
                ZoneScopedN("IrradianceCompute/Execute");
                TracyVkZone(context.tracy, context.command, "IrradianceCompute");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                m_Resources.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.pipeline);

                // to constexpr the ceil (TODO cpp23 makes ceil constexpr)
                constexpr std::uint32_t groupSize = 8;
                constexpr glm::uvec2 groupCount = {
                    (resolution.x + groupSize - 1) / groupSize,
                    (resolution.y + groupSize - 1) / groupSize,
                };

                context.command.dispatch(groupCount.x, groupCount.y, 1);
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

        success &= App::LayoutAllocator.write(
            m_Resources.layout,
            m_IrradianceMap,
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = Image::RemainingMipLevels,
                    .layerCount = 1,
                },
            },
            2);
    }

    return success;
}

bool IrradianceCompute::destroy() {
    return true;
}

const RenderGraph &IrradianceCompute::getGraph() const {
    return m_Graph;
}

const Resource<Texture> &IrradianceCompute::getIrradianceMap() const {
    return m_IrradianceMap;
}

const IrradianceCompute::Info &IrradianceCompute::getInfo() const {
    return m_Info;
}

bool IrradianceCompute::buildResources() {
    auto layout = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/irradiance",
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
                    // irradiance map
                    .type = DescriptorLayout::Type::eStorageImage,
                    .stage = DescriptorLayout::ShaderStageFlags::eCompute,
                },
            },
            .lifetime = DescriptorLayout::Lifetime::Global,
        }});

    if (!layout) {
        return false;
    }

    auto pipeline = ResourceRegistry<ComputePipeline>::insert(
        "builtin/irradiance",
        {
            {
                .module = "builtin/compute/irradiance",
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
