#pragma once

#include "../ecs/system.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/rendergraph.hpp"
#include "../resources/resource.hpp"
#include "../window/window.hpp"

namespace Physbuzz {

class RenderPipeline;
class PipelineLayout;
class Mesh;

namespace Builtin {

namespace RenderPipelineSkybox {

struct VertexSkybox {
    glm::vec3 position;

    static VertexDescription Description;
};

inline Resource<Mesh> ResourceMesh = {"builtin/skybox"};

inline Resource<PipelineLayout> ResourceLayoutFrame = {"builtin/skybox/frame"};
inline Resource<PipelineLayout> ResourceLayoutTexture = {"builtin/skybox/texture"};

inline Resource<RenderPipeline> Resource = {"builtin/skybox"};

bool build(const std::shared_ptr<Transfer> transfer);

} // namespace RenderPipelineSkybox

} // namespace Builtin

class SkyboxRenderer : public System<> {
  public:
    constexpr static RenderNodeID Output = "builtin/skybox";

    struct Info {
        ObjectID camera;
        std::shared_ptr<Window> window;

        Resource<Texture> skybox;
        Resource<RenderPipeline> pipeline = Builtin::RenderPipelineSkybox::Resource;
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
