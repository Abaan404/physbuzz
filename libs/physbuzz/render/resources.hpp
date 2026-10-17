#pragma once

#include "../ecs/defines.hpp"
#include "../ecs/system.hpp"
#include "../graphics/rendergraph.hpp"
#include "../resources/registry.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

class DynamicBuffer;
class CameraComponent;
class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;
class Scene;
struct RenderContext;

class ResourceContext : public System<> {
  public:
    struct CameraBufferData {
        alignas(16) glm::vec3 position;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };

    struct DirectionalLightBufferData {
        glm::mat4 projectionView;

        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 intensity;
    };

    struct PointLightBufferData {
        std::array<glm::mat4, 6> projectionView;

        glm::vec3 position;
        float depth;
        alignas(16) glm::vec3 intensity;
    };

    struct SpotLightBufferData {
        glm::mat4 projectionView;

        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 intensity;

        float cutOff;
        float outerCutOff;
    };

    struct Resources {
        Resource<DynamicBuffer> camera;
        Resource<DynamicBuffer> directionalLights;
        Resource<DynamicBuffer> pointLights;
        Resource<DynamicBuffer> spotLights;
    };

    struct Info {
        ObjectID cameraId;
    };

    ResourceContext(Scene *scene);

    bool build() override;
    bool destroy() override;

    RenderNode buildCameraNode();
    RenderNode buildLightsNode();

    void setCamera(ObjectID camera);

    const Info &getInfo() const;
    const Resources &getResouces() const;

  private:
    Info m_Info;

    Resources m_Resources;

    std::vector<DirectionalLightBufferData> directionalLightsBuffer;
    std::vector<PointLightBufferData> pointLightsBuffer;
    std::vector<SpotLightBufferData> spotLightsBuffer;
};

} // namespace Physbuzz
