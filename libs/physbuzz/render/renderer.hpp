#pragma once

#include "../ecs/system.hpp"
#include "buffer.hpp"
#include "renderers/deferred.hpp"
#include "renderers/forward.hpp"
#include "shadow.hpp"

namespace Physbuzz {

class PipelineLayoutAllocator;

class UniformBuffer;
class StorageBuffer;

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
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

struct Model {
    alignas(16) glm::mat4 model;
};

inline Resource<UniformBuffer> CameraBuffer = {"builtin/renderer/camera"};
inline Resource<StorageBuffer> ModelBuffer = {"builtin/renderer/model"};

inline Resource<PipelineLayout> Resource = {"builtin/renderer"};

bool build(const std::shared_ptr<PipelineLayoutAllocator> allocator);

} // namespace LayoutRenderer

} // namespace Builtin

class Window;

class Renderer : public System<> {
  public:
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
    std::uint32_t getFrameInFlight() const;

  private:
    std::shared_ptr<IRenderer> getRenderer() const;
    bool buildSystems();
    bool destroySystems();

    Info m_Info;

    std::uint32_t m_FrameInFlight = 0;

    struct {
        Image image = {{
            .usage = Image::ImageUsageFlagBits::eDepthStencilAttachment,
            .type = Image::Type::e2D,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = Image::Format::eD32Sfloat,
        }};
        vk::ImageView view;
    } m_Depth;

    struct {
        vk::CommandPool pool = nullptr;
        std::vector<vk::CommandBuffer> buffers = {};
    } m_Command = {};

    struct {
        std::array<vk::Semaphore, detail::MAX_FRAMES_IN_FLIGHT> presentComplete = {};
        std::vector<vk::Semaphore> renderFinished = {};
    } m_Semaphores = {};

    struct {
        std::array<vk::Fence, detail::MAX_FRAMES_IN_FLIGHT> inFlight = {};
    } m_Fences = {};

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz
