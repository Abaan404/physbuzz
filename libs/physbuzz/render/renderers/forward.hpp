#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/resources.hpp"
#include "../lighting.hpp"
#include "defines.hpp"

namespace Physbuzz {

class StaticBuffer;
class PipelineLayout;
class RenderPipeline;

namespace Builtin {

namespace RenderPipelineForward {

struct CameraBuffer {
    alignas(16) glm::vec3 position;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

struct LightBuffer {
    std::array<DirectionalLightComponent, 5> directionals;
    std::array<PointLightComponent, 5> points;
    std::array<SpotLightComponent, 5> spots;
};

struct MaterialBuffer {
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    float specularity;
};

struct ModelBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 invModel;
};

inline Resource<StaticBuffer> ResourceBufferCamera = {"builtin/forward/camera"};
inline Resource<StaticBuffer> ResourceBufferLight = {"builtin/forward/light"};

inline Resource<StaticBuffer> ResourceBufferMaterial = {"builtin/forward/material"};
inline Resource<StaticBuffer> ResourceBufferModel = {"builtin/forward/model"};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/forward/frame"};
inline Resource<PipelineLayout> ResourceLayoutObject = {"builtin/forward/object"};

inline Resource<RenderPipeline> Resource = {"builtin/forward"};

bool build();

} // namespace RenderPipelineForward

} // namespace Builtin

class ForwardRenderer : public IRenderPass,
                        public System<RenderComponent> {
  public:
    struct Info {
        ObjectID camera;
    };

    ForwardRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void render(const RenderContext &context) override;

  private:
    Info m_Info;

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
