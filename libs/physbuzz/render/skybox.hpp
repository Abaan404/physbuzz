#pragma once

#include "../ecs/system.hpp"
#include "../graphics/rendergraph.hpp"
#include "../resources/registry.hpp"
#include "../window/window.hpp"
#include "resources.hpp"

namespace Physbuzz {

class DescriptorLayout;

class SkyboxRenderer : public System<> {
  public:
    inline static RenderNodeID Output = "builtin/skybox";

    struct Info {
        std::shared_ptr<Window> window;

        Resource<Texture> skybox;
    };

    struct Resources {
        struct Specialization {
            std::uint32_t isCubemap;
        };

        Resource<DescriptorLayout> layoutTexture;
        Resource<DescriptorLayout> layoutFrame;
        Resource<GraphicsPipeline> pipeline;
    };

    SkyboxRenderer(Scene *scene, const Info &info);

    bool build() override;
    bool destroy() override;

    const RenderGraph &getGraph() const;
    const Resources &getResources() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    Resources m_Resources;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        EventID resize = -1;
    } m_Events = {};

    bool buildResources();
};

} // namespace Physbuzz
