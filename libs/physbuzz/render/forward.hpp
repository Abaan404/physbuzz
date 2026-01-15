#pragma once

#include "../ecs/system.hpp"
#include "../resources/table.hpp"
#include "defines.hpp"
#include "resources/common.hpp"

namespace Physbuzz {

class RenderPipeline;

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
