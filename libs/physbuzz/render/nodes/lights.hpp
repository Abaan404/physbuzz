#pragma once

#include "../../graphics/rendergraph.hpp"
#include "../../resources/resource.hpp"

namespace Physbuzz {

namespace Builtin {

namespace RenderNodeLights {

struct DirectionalLightBuffer {
    glm::mat4 projectionView;

    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 intensity;
};

struct PointLightBuffer {
    glm::mat4 projectionView;

    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 intensity;
};

struct SpotLightBuffer {
    glm::mat4 projectionView;

    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 intensity;

    float cutOff;
    float outerCutOff;
};

inline Resource<DynamicBuffer> ResourceBufferDirectional = {"builtin/lights/directional"};
inline Resource<DynamicBuffer> ResourceBufferPoint = {"builtin/lights/point"};
inline Resource<DynamicBuffer> ResourceBufferSpot = {"builtin/lights/spot"};

RenderNode build();

} // namespace RenderNodeLights

} // namespace Builtin

} // namespace Physbuzz
