#pragma once

#include "../ecs/system.hpp"
#include "../graphics/rendergraph.hpp"
#include "../resources/resource.hpp"
#include "../window/window.hpp"

namespace Physbuzz {

class GraphicsPipeline;
class DescriptorLayout;
class Texture;
class Mesh;

namespace Builtin {

namespace PipelineSkybox {

struct Specialization {
    std::uint32_t isCubemap;
};

inline Resource<DescriptorLayout> ResourceLayoutTexture = {"builtin/skybox/texture"};
inline Resource<DescriptorLayout> ResourceLayoutFrame = {"builtin/skybox/frame"};

inline Resource<GraphicsPipeline> Resource = {"builtin/skybox"};

bool build();

} // namespace PipelineSkybox

} // namespace Builtin

class SkyboxRenderer : public System<> {
  public:
    constexpr static RenderNodeID Output = "builtin/skybox";

    struct Info {
        ObjectID camera;
        std::shared_ptr<Window> window;

        Resource<Texture> skybox;
    };

    SkyboxRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    const RenderGraph &getGraph() const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    RenderGraph m_Graph = {{
        .output = Output,
    }};

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
