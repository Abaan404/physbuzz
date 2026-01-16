#include "forward.hpp"

#include "../../graphics/descriptors/dynamic.hpp"
#include "../../graphics/layout.hpp"
#include "../lighting.hpp"
#include "../model.hpp"
#include "common.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineForward::build() {
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
            .module = "builtin/forward",
            .description = &Model::Vertex::Description,
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
