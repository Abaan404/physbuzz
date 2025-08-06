#pragma once

#include "../ecs/system.hpp"
#include "renderers/deferred.hpp"
#include "renderers/forward.hpp"
#include "shadow.hpp"
#include "uniforms.hpp"

namespace Physbuzz {

namespace Builtin {

namespace VertexRendererScreenQuad {

struct Format {
    glm::vec3 position;
    glm::vec2 texCoords;
};

inline Resource<VertexAttribute> Resource = {"builtin/renderer/screenquad"};

bool build();

} // namespace VertexRendererScreenQuad

namespace MeshRendererScreenQuad {

inline Resource<Model> Resource = {"builtin/renderer/screenquad"};

bool build();

} // namespace MeshRendererScreenQuad

namespace ShaderRendererPassthrough {

inline Resource<ShaderPipeline> Resource = {"builtin/renderer/passthrough"};

bool build();

} // namespace ShaderRendererPassthrough

namespace UniformRendererCamera {

struct Format {
    glm::vec3 position;
    float _padding0;
    glm::mat4x4 view;
    glm::mat4x4 projection;
};

constexpr std::uint32_t Binding = 0;

inline Resource<UniformBuffer<Format>> Resource = {"builtin/renderer/camera"};

bool build();

} // namespace UniformRendererCamera

} // namespace Builtin

class Window;

class Renderer : public System<> {
  public:
    enum class Type {
        Deferred,
        Forward,
    };

    struct Info {
        bool passthrough = true;
        Type type = Type::Deferred;
        ObjectID camera = -1;
        glm::ivec2 resolution = {1, 1};

        ForwardRenderer::Info forward = {};
        DeferredRenderer::Info deferred = {};
        Shadow::Info shadow = {};

        std::vector<Resource<ShaderPipeline>> postProcessing = {};
    };

    Renderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void tick();

    void resize(const glm::ivec2 &resolution);

    const Type &getType();
    void setType(const Type &type);

    const Framebuffer &getFramebuffer() const;
    const Info &getInfo() const;

  private:
    std::shared_ptr<IRenderer> getRenderer() const;
    bool buildRenderer();
    bool destroyRenderer();

    Info m_Info;

    struct {
        EventID resize;
    } m_Events;
};

} // namespace Physbuzz
