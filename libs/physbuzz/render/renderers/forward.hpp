#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/table.hpp"
#include "../lighting.hpp"
#include "defines.hpp"

namespace Physbuzz {

class DynamicBuffer;
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
    std::array<std::uint32_t, 5> diffuseTextureIds;
    std::array<std::uint32_t, 5> specularTextureIds;
    float specularity;
};

struct ModelBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 invModel;
    std::uint64_t materialOffset;
};

struct PushConstants {
    std::uint64_t material;
};

inline Resource<StaticBuffer> ResourceBufferMaterials = {"builtin/forward/materials"};
inline Resource<DynamicBuffer> ResourceBufferTextures = {"builtin/forward/textures"};

inline Resource<DynamicBuffer> ResourceBufferCamera = {"builtin/forward/camera"};
inline Resource<DynamicBuffer> ResourceBufferLight = {"builtin/forward/light"};

inline Resource<DynamicBuffer> ResourceBufferModel = {"builtin/forward/models"};

inline Resource<PipelineLayout> ResourceLayoutGlobal = {"builtin/forward/global"};
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

        Resource<RenderPipeline> pipeline = Builtin::RenderPipelineForward::Resource;
    };

    ForwardRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void render(const RenderContext &context) override;

  private:
    Info m_Info;

    bool m_ReloadedPipeline = false;

    ResourceTable<Texture> m_Textures;
    ResourceTable<Material> m_Materials;

    struct {
        EventID resize = -1;
        EventID pipelineReload = -1;
    } m_Events = {};
};

} // namespace Physbuzz
