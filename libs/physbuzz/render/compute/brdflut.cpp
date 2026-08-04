#include "brdflut.hpp"

#include "../../app/application.hpp"
#include "../../graphics/layout.hpp"
#include "../../graphics/material.hpp"
#include "../../graphics/pipeline.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

namespace Builtin {

bool PipelineBrdfLut::build() {
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
                        // lut map
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
                .module = "builtin/compute/brdf_lut",
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

BrdfLutCompute::BrdfLutCompute() {}

bool BrdfLutCompute::build() {
    // build pipeline
    if (!Builtin::PipelineBrdfLut::build()) {
        Logger::ERROR("[BrdfLutCompute] Could not build brdf lut pipeline.");
        return false;
    }

    bool success = true;

    constexpr glm::vec3 resolution = {512, 512, 1};

    success &= ResourceRegistry<Texture>::insert(
        Builtin::PipelineBrdfLut::ResourceLut,
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
                    .output = {{
                        Builtin::PipelineBrdfLut::ResourceLut,
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
            .execute = [this](Scene *scene, const RenderContext &context) {
                ZoneScopedN("BrdfLutCompute/Execute");
                TracyVkZone(context.tracy, context.command, "BrdfLutCompute");

                std::lock_guard<std::mutex> lock(ResourceRegistry<ComputePipeline>::ReloadMutex);

                Builtin::PipelineBrdfLut::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::PipelineBrdfLut::Resource);

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
            Builtin::PipelineBrdfLut::ResourceLayout,
            Builtin::PipelineBrdfLut::ResourceLut,
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

} // namespace Physbuzz
