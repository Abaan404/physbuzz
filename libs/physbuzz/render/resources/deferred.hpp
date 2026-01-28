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

namespace Geometry {

struct ModelBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 invModel;
    std::uint32_t materialIdx;
};

struct PushConstants {
    std::uint64_t materialBaseAddress;
};

inline Resource<DynamicBuffer> ResourceBufferModel = {"builtin/deferred/models"};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/deferred/geometry/frame"};
inline Resource<PipelineLayout> ResourceLayoutObject = {"builtin/deferred/geometry/object"};

inline Resource<RenderPipeline> Resource = {"builtin/deferred/geometry"};

bool build();

} // namespace Geometry

namespace Lighting {

struct PushConstants {
    std::uint32_t directionalCount;
    std::uint32_t spotCount;
    std::uint32_t pointCount;
};

inline Resource<DynamicBuffer> ResourceBufferDirectionalLights = {"builtin/deferred/lighting/directionals"};
inline Resource<DynamicBuffer> ResourceBufferPointLights = {"builtin/deferred/light/points"};
inline Resource<DynamicBuffer> ResourceBufferSpotLights = {"builtin/deferred/light/spots"};

inline Resource<PipelineLayout> ResourceLayoutGBuffer = {"builtin/deferred/lighting/gBuffer"};
inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/deferred/lighting/frame"};

inline Resource<RenderPipeline> Resource = {"builtin/deferred/lighting"};

bool build();

} // namespace Lighting

struct CameraBuffer {
    alignas(16) glm::vec3 position;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

inline Resource<DynamicBuffer> ResourceBufferCamera = {"builtin/deferred/camera"};

inline std::array ResourceTextureGBuffers = {
    Resource<Texture>{"builtin/deferred/gBuffer0"},
    Resource<Texture>{"builtin/deferred/gBuffer1"},
    Resource<Texture>{"builtin/deferred/gBuffer2"},
};

bool build();

} // namespace RenderPipelineDeferred

} // namespace Builtin

} // namespace Physbuzz
