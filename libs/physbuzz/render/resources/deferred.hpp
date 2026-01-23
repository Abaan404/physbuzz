#pragma once

#include "../../resources/resource.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

class DynamicBuffer;
class Texture;
class PipelineLayout;
class RenderPipeline;

namespace Builtin {

namespace RenderPipelineDeferred {

struct CameraBuffer {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

struct ModelBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 invModel;
    std::uint32_t materialIdx;
};

struct PushConstants {
    std::uint64_t materialBaseAddress;
};

inline Resource<DynamicBuffer> ResourceBufferCamera = {"builtin/deferred/camera"};

inline std::array ResourceTextureGBuffers = {
    Resource<Texture>{"builtin/deferred/gbuffer0"},
    Resource<Texture>{"builtin/deferred/gbuffer1"},
    Resource<Texture>{"builtin/deferred/gbuffer2"},
};

inline Resource<DynamicBuffer> ResourceBufferModel = {"builtin/deferred/models"};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/deferred/frame"};
inline Resource<PipelineLayout> ResourceLayoutObject = {"builtin/deferred/object"};

inline Resource<RenderPipeline> ResourceGeometry = {"builtin/deferred/geometry"};
inline Resource<RenderPipeline> ResourceLighting = {"builtin/deferred/lighting"};

bool build();

} // namespace RenderPipelineDeferred

} // namespace Builtin

} // namespace Physbuzz
