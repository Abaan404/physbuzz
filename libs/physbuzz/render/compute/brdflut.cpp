#include "brdflut.hpp"

#include "../../app/application.hpp"
#include "../../graphics/layout.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/pipeline.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

BrdfLutCompute::BrdfLutCompute() {}

bool BrdfLutCompute::build() {
    if (!buildResources()) {
        Logger::ERROR("[BrdfLutCompute] Could not build brdf lut pipeline.");
        return false;
    }

    bool success = true;

    constexpr glm::vec3 resolution = {512, 512, 1};

    auto lut = ResourceRegistry<Texture>::insert(
        "builtin/brdf_lut",
        {{
            .type = Texture::Type::Dim2D,
            .usage = Texture::Usage::Storage,
            .sampler = {{Sampler::Type::Linear}},
        }},
        resolution);

    if (!lut) {
        return false;
    }

    m_Lut = std::move(*lut);

    m_Graph.add(
        Output,
        {
            .description = {
                .textures = {
                    .output = {{
                        m_Lut,
                        {
                            .stage = RenderNode::Stage::Compute,
                            .subresourceRanges = {{
                                .aspectMask = Image::AspectFlags::eColor,
                                .levelCount = 1,
                                .layerCount = 1,
                            }},
                        },
                    }},
                },
            },
            .execute = [this](const RenderContext &context) {
                ZoneScopedN("BrdfLutCompute/Execute");
                TracyVkZone(context.tracy, context.command, "BrdfLutCompute");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                m_Resources.pipeline->bind(context);
                App::LayoutAllocator.bind(context, m_Resources.pipeline);

                std::uint32_t groupSize = 8;
                glm::uvec2 groupCount = {
                    (resolution.x + groupSize - 1) / groupSize,
                    (resolution.y + groupSize - 1) / groupSize,
                };

                context.command.dispatch(groupCount.x, groupCount.y, 1);
            },
        });

    success &= m_Graph.compile();

    if (success) {
        success &= App::LayoutAllocator.write(
            m_Resources.layout,
            m_Lut,
            Image::ViewInfo{
                .type = Image::ViewType::e2D,
                .subresourceRange = {
                    .aspectMask = Image::AspectFlags::eColor,
                    .levelCount = Image::RemainingMipLevels,
                    .layerCount = 1,
                },
            },
            0);
    }

    return success;
}

bool BrdfLutCompute::destroy() {
    return true;
}

const RenderGraph &BrdfLutCompute::getGraph() const {
    return m_Graph;
}

const Resource<Texture> &BrdfLutCompute::getLut() const {
    return m_Lut;
}

bool BrdfLutCompute::buildResources() {
    auto layout = ResourceRegistry<DescriptorLayout>::insert(
        "builtin/brdf_lut",
        {{
            .bindings = {
                {
                    // lut map
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
        "builtin/brdf_lut",
        {
            {
                .module = "builtin/compute/brdf_lut",
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
