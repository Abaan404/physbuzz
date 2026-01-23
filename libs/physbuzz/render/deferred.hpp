#pragma once

#include "../ecs/system.hpp"
#include "../graphics/defines.hpp"
#include "../graphics/pipeline.hpp"
#include "../resources/resource.hpp"
#include "defines.hpp"
#include "resources/deferred.hpp"

namespace Physbuzz {

class DeferredRenderer : public IRenderPass,
                         public System<RenderComponent> {
  public:
    struct Info {
        ObjectID camera;

        Resource<RenderPipeline> geometry = Builtin::RenderPipelineDeferred::ResourceGeometry;
        Resource<RenderPipeline> lighting = Builtin::RenderPipelineDeferred::ResourceLighting;
    };

    DeferredRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void render(const RenderContext &context) override;

    const Info &getInfo() const;

  private:
    Info m_Info;

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
