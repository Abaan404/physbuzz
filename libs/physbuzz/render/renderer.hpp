#pragma once

#include "../ecs/system.hpp"
#include "buffer.hpp"
#include "renderers/defines.hpp"
#include "shadow.hpp"
#include <memory>

namespace Physbuzz {

class Window;

class Renderer : public System<> {
  public:
    struct Info {
        Shadow::Info shadow = {};

        std::shared_ptr<Window> window;
        std::vector<Resource<RenderPipeline>> postProcessing = {};
    };

    Renderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void tick();
    void immediate(std::function<void(const vk::CommandBuffer &)> record);

    void setRenderPasses(const std::vector<std::shared_ptr<IRenderPass>> &renderpasses);
    const Info &getInfo() const;

  private:
    void resize(const glm::ivec2 &resolution);

    Info m_Info;

    std::vector<std::shared_ptr<IRenderPass>> m_RenderPasses;
    std::uint32_t m_FrameInFlight = 0;

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
        EventID resize = -1;
    } m_Events = {};

    friend class StorageBuffer;
    friend class UniformBuffer;
    friend class PipelineLayoutAllocator;
};

} // namespace Physbuzz
