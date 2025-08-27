#pragma once

#include "../ecs/system.hpp"
#include "renderers/deferred.hpp"
#include "renderers/forward.hpp"
#include "shadow.hpp"

namespace Physbuzz {

namespace Builtin {

namespace MeshRendererScreenQuad {

inline Resource<Model> Resource = {"builtin/renderer/screenquad"};

bool build();

} // namespace MeshRendererScreenQuad

namespace ShaderRendererPassthrough {

inline Resource<RenderPipeline> Resource = {"builtin/renderer/passthrough"};

bool build();

} // namespace ShaderRendererPassthrough

namespace LayoutRenderer {

struct Camera {
    alignas(16) glm::vec3 position;
    alignas(16) glm::mat4x4 view;
    alignas(16) glm::mat4x4 projection;
    static constexpr std::uint32_t Binding = 0;
};

inline Resource<PipelineLayout> Resource = {"builtin/renderer/layout"};

bool build();

} // namespace LayoutRenderer

} // namespace Builtin

class Window;

class Renderer : public System<> {
  public:
    struct VertexScreenQuad {
        glm::vec3 position;
        glm::vec2 texCoords;

        static VertexDescription Description;
    };

    enum class Type {
        Deferred,
        Forward,
    };

    struct Info {
        Type type = Type::Deferred;
        ObjectID camera = -1;

        ForwardRenderer::Info forward = {};
        DeferredRenderer::Info deferred = {};
        Shadow::Info shadow = {};

        std::shared_ptr<Window> window;
        std::vector<Resource<RenderPipeline>> postProcessing = {};
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
    bool buildSystems();
    bool destroySystems();

    Info m_Info;

    struct Frames {
        std::uint32_t inFlight = 0;
        static constexpr std::uint32_t MAX_IN_FLIGHT = 2;
    } m_Frame;

    struct {
        vk::CommandPool pool = nullptr;
        std::vector<vk::CommandBuffer> buffers = {};
    } m_Command = {};

    struct {
        std::array<vk::Semaphore, Frames::MAX_IN_FLIGHT> presentComplete = {};
        std::array<vk::Semaphore, Frames::MAX_IN_FLIGHT> renderFinished = {};
    } m_Semaphores = {};

    struct {
        std::array<vk::Fence, Frames::MAX_IN_FLIGHT> inFlight = {};
    } m_Fences = {};

    struct {
        EventID resize = -1;
    } m_Events = {};

    friend class PipelineLayoutAllocator;
};

} // namespace Physbuzz
