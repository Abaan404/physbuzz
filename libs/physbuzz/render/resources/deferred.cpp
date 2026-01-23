#include "deferred.hpp"

#include "../../graphics/descriptors/dynamic.hpp"
#include "../../graphics/layout.hpp"
#include "../model.hpp"
#include "common.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineDeferred::build() {
    if (ResourceRegistry<RenderPipeline>::contains(ResourceGeometry)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<PipelineLayout>::contains(RenderLayoutGlobal::Resource)) {
        success &= RenderLayoutGlobal::build();
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // camera
                        .type = PipelineLayout::Type::eUniformBuffer,
                        .range = sizeof(CameraBuffer),
                    },
                },
            }});
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutObject)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutObject,
            {{
                .bindings = {
                    {
                        // instance
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferCamera)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferCamera,
            {{
                .type = DynamicBuffer::Type::Constant,
            }},
            sizeof(CameraBuffer));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferModel)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferModel,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(ModelBuffer));
    }

    for (const auto &gBuffer : ResourceTextureGBuffers) {
        if (!ResourceRegistry<Texture>::contains(gBuffer)) {
            success &= ResourceRegistry<Texture>::insert(
                gBuffer,
                {{
                    .type = Texture::Type::Attachment,
                    .sampler = Texture::Sampler::None,
                    .format = Texture::Format::eR8G8B8A8Srgb,
                }},
                glm::uvec3{1, 1, 1});
        }
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        ResourceGeometry,
        {{
            .module = "builtin/deferred/geometry",
            .description = &Model::Vertex::Description,
            .blend = {
                .attachments = {4, {{}}},
            },
            .formats = {
                .color = {
                    RenderPipeline::Format::eR8G8B8A8Srgb,
                    RenderPipeline::Format::eR8G8B8A8Srgb,
                    RenderPipeline::Format::eR8G8B8A8Srgb,
                    RenderPipeline::Format::eR8G8B8A8Srgb,
                },
            },
            .layouts = {
                .resources = {
                    RenderLayoutGlobal::Resource,
                    ResourceLayoutFrame,
                    ResourceLayoutObject,
                },
                .pushConstantRanges = {
                    {
                        .stageFlags = RenderPipeline::PushConstantsStageFlags::eAll,
                        .size = sizeof(PushConstants),
                    },
                },
            },
        }});

    return success;
}

} // namespace Builtin

} // namespace Physbuzz
