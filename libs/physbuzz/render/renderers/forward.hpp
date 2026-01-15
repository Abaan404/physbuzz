#pragma once

#include "../../ecs/system.hpp"
#include "../../resources/table.hpp"
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

struct MaterialBuffer {
    std::uint32_t diffuseTextureId;
    std::uint32_t specularTextureId;
    float specularity;
};

struct ModelBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 invModel;
    std::uint32_t materialIdx;
};

struct PushConstants {
    std::uint32_t directionalCount;
    std::uint32_t spotCount;
    std::uint32_t pointCount;

    std::uint64_t materialBaseAddress;
};

inline Resource<StaticBuffer> ResourceBufferMaterials = {"builtin/forward/materials"};
inline Resource<DynamicBuffer> ResourceBufferTextures = {"builtin/forward/textures"};

inline Resource<DynamicBuffer> ResourceBufferCamera = {"builtin/forward/camera"};
inline Resource<DynamicBuffer> ResourceBufferDirectionalLights = {"builtin/forward/light/directionals"};
inline Resource<DynamicBuffer> ResourceBufferPointLights = {"builtin/forward/light/points"};
inline Resource<DynamicBuffer> ResourceBufferSpotLights = {"builtin/forward/light/spots"};

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
    } m_Events = {};
};

} // namespace Physbuzz
