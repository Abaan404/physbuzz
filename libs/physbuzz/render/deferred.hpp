#pragma once

#include "../ecs/system.hpp"
#include "../graphics/pipeline.hpp"
#include "../resources/resource.hpp"
#include "defines.hpp"

namespace Physbuzz {

class Texture;

namespace Builtin {

namespace RenderPipelineDeferred {

inline Resource<Texture> ResourceTextureAlbedo = {"builtin/forward/materials"};

inline Resource<RenderPipeline> ResourceGeometry = {"builtin/deferred/geometry"};
inline Resource<RenderPipeline> ResourceLighting = {"builtin/deferred/lighting"};

bool build();

} // namespace RenderPipelineDeferred

} // namespace Builtin

struct RenderComponent;

struct DeferredRenderComponent {
    struct ForwardPass {
        Resource<RenderPipeline> pipeline;
    };
};

class DeferredRenderer : public IRenderPass,
                         public System<RenderComponent, DeferredRenderComponent> {
  public:
    struct Info {
        ObjectID camera;

        struct {
            Resource<RenderPipeline> geometry = Builtin::RenderPipelineDeferred::ResourceGeometry;
            Resource<RenderPipeline> lighting = Builtin::RenderPipelineDeferred::ResourceLighting;
        } passes = {};
    };

    DeferredRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void render(const RenderContext &context) override;

    const Info &getInfo() const;

  private:
    Info m_Info;
};

} // namespace Physbuzz
