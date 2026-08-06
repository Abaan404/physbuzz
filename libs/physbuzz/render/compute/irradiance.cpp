#include "irradiance.hpp"

#include "../../app/application.hpp"
#include "../../graphics/layout.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/pipeline.hpp"
#include "physbuzz/resources/registry.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool PipelineIrradiance::build() {
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
    }

    success &= ResourceRegistry<ComputePipeline>::insert(
        Resource,
        {
            {
                .module = "builtin/compute/irradiance",
                .specialization = {
                    .offsets = {
                        offsetof(Specialization, isCubemap),
                    },
                    .size = sizeof(Specialization),
                },
            },
            {
                .layouts = {
                    .resources = {
                        ResourceLayout,
                    },
                },
            },
        });

    return success;
}

} // namespace Builtin

IrradianceCompute::IrradianceCompute(const Info &info)
    : m_Info(info) {}

bool IrradianceCompute::build() {
    // build pipeline
    if (!Builtin::PipelineIrradiance::build()) {
        Logger::ERROR("[IrradianceCompute] Could not build irradiance pipeline.");
        return false;
    }

    bool success = true;

    Builtin::PipelineIrradiance::Specialization specialization = {
        .isCubemap = m_Info.environmentMap->getInfo().type == Texture::Type::Cube,
    };

    success &= Builtin::PipelineIrradiance::Resource->specialize(specialization);

    constexpr glm::vec3 resolution = {128, 64, 1};

    success &= ResourceRegistry<Texture>::insert(
        m_Resource,
        {{
            .type = Texture::Type::Dim2D,
            .usage = Texture::Usage::Storage,
            .sampler = {{Sampler::Type::Linear}},
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
                            .subresourceRanges = {{
                                .aspectMask = Image::AspectFlags::eColor,
                                .levelCount = Image::RemainingMipLevels,
                                .layerCount = 1,
                            }},
                        },
                    }},
                    .output = {{
                        m_Resource,
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
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("IrradianceCompute/Execute");
                TracyVkZone(context.tracy, context.command, "IrradianceCompute");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                Builtin::PipelineIrradiance::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineIrradiance::Resource);

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
                Builtin::PipelineIrradiance::ResourceLayout,
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
                Builtin::PipelineIrradiance::ResourceLayout,
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
            Builtin::PipelineIrradiance::ResourceLayout,
            m_Resource,
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
    ResourceRegistry<Texture>::erase(m_Resource);

    return true;
}

const RenderGraph &IrradianceCompute::getGraph() const {
    return m_Graph;
}

const Resource<Texture> &IrradianceCompute::getIrradianceMap() const {
    return m_Resource;
}

const IrradianceCompute::Info &IrradianceCompute::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
