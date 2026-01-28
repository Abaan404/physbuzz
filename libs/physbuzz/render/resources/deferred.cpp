#include "deferred.hpp"

#include "../../graphics/descriptors/dynamic.hpp"
#include "../../graphics/layout.hpp"
#include "../lighting.hpp"
#include "../model.hpp"
#include "common.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineDeferred::Geometry::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
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

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferModel)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferModel,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(ModelBuffer));
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/deferred/geometry",
            .description = &Model::Vertex::Description,
            .blend = {
                .attachments = {4, {{}}},
            },
            .formats = {
                .color = {
                    RenderPipeline::Format::eR8G8B8A8Srgb,
                    RenderPipeline::Format::eR8G8B8A8Snorm,
                    RenderPipeline::Format::eR8G8B8A8Snorm,
                    RenderPipeline::Format::eR8G8B8A8Snorm,
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
            .attachments = {
                .colors = {vk::AttachmentUnused, 0, 1, 2},
            },
        }});

    return success;
}

bool RenderPipelineDeferred::Lighting::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

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
                    {
                        // directionals
                        .type = PipelineLayout::Type::eStorageBuffer,
                        .range = sizeof(DirectionalLightComponent),
                    },
                    {
                        // points
                        .type = PipelineLayout::Type::eStorageBuffer,
                        .range = sizeof(PointLightComponent),
                    },
                    {
                        // spots
                        .type = PipelineLayout::Type::eStorageBuffer,
                        .range = sizeof(SpotLightComponent),
                    },
                },
            }});
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutGBuffer)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutGBuffer,
            {{
                .bindings = {
                    {
                        .type = PipelineLayout::Type::eInputAttachment,
                        .stage = PipelineLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        .type = PipelineLayout::Type::eInputAttachment,
                        .stage = PipelineLayout::ShaderStageFlags::eFragment,
                    },
                    {
                        .type = PipelineLayout::Type::eInputAttachment,
                        .stage = PipelineLayout::ShaderStageFlags::eFragment,
                    },
                },
                .lifetime = PipelineLayout::Lifetime::Global,
            }});
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferDirectionalLights)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferDirectionalLights,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(DirectionalLightComponent));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferPointLights)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferPointLights,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(PointLightComponent));
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferSpotLights)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferSpotLights,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(SpotLightComponent));
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/deferred/lighting",
            .rasterization = {
                .cullMode = RenderPipeline::CullModeFlags::eNone,
            },
            .blend = {
                .attachments = {4, {{}}},
            },
            .formats = {
                .color = {
                    RenderPipeline::Format::eR8G8B8A8Srgb,
                    RenderPipeline::Format::eR8G8B8A8Snorm,
                    RenderPipeline::Format::eR8G8B8A8Snorm,
                    RenderPipeline::Format::eR8G8B8A8Snorm,
                },
            },
            .layouts = {
                .resources = {
                    ResourceLayoutGBuffer,
                    ResourceLayoutFrame,
                },
                .pushConstantRanges = {
                    {
                        .stageFlags = RenderPipeline::PushConstantsStageFlags::eAll,
                        .size = sizeof(PushConstants),
                    },
                },
            },
            .attachments = {
                .colors = {vk::AttachmentUnused, 0, 1, 2},
            },
        }});

    return success;
}

bool RenderPipelineDeferred::build() {
    bool success = true;

    if (!ResourceRegistry<RenderPipeline>::contains(Geometry::Resource)) {
        success &= Geometry::build();
    }

    if (!ResourceRegistry<RenderPipeline>::contains(Lighting::Resource)) {
        success &= Lighting::build();
    }

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceBufferCamera)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceBufferCamera,
            {{
                .type = DynamicBuffer::Type::Constant,
            }},
            sizeof(CameraBuffer));
    }

    for (const auto &gBuffer : ResourceTextureGBuffers) {
        if (!ResourceRegistry<Texture>::contains(gBuffer)) {
            success &= ResourceRegistry<Texture>::insert(
                gBuffer,
                {{
                    .type = Texture::Type::Attachment,
                    .sampler = Texture::Sampler::None,
                    .format = Texture::Format::eR8G8B8A8Snorm,
                }},
                glm::uvec3{1, 1, 1});
        }
    }

    return success;
}

} // namespace Builtin

} // namespace Physbuzz
