#pragma once

#include "../app/deletion.hpp"
#include "../ecs/system.hpp"
#include "../window/window.hpp"
#include "defines.hpp"
#include "material.hpp"
#include "memory.hpp"
#include "rendergraph.hpp"
#include <memory>

namespace Physbuzz {

class Window;

class Renderer : public System<> {
  public:
    struct Info {
        std::shared_ptr<Window> window;
    };

    Renderer(const Info &info, const RenderGraph &graph);

    bool build() override;
    bool destroy() override;

    void tick();
    void immediate(const std::function<void(vk::CommandBuffer)> &record);

    void setGraph(const RenderGraph &graph);

    bool map(const Buffer &buffer, const std::span<const std::byte> &bytes, std::uint64_t offset);
    bool map(const Image &image, const std::span<const std::byte> &bytes);

    const RenderGraph &getGraph() const;
    const Info &getInfo() const;
    std::uint32_t getFrameInFlight() const;

  private:
    void resize(const glm::ivec2 &resolution);

    Info m_Info;

    RenderGraph m_Graph;
    MaterialAllocator m_MaterialAllocator;

    std::uint32_t m_FrameInFlight = 0;

    std::array<DeletionQueue, detail::MAX_FRAMES_IN_FLIGHT> m_DeletionQueues;

    Attachment m_Depth = {{
        .usage = Attachment::Usage::DepthStencil,
        .format = Attachment::Format::eD32SfloatS8Uint,
    }};

    struct {
        vk::CommandPool pool = nullptr;
        std::array<vk::CommandBuffer, detail::MAX_FRAMES_IN_FLIGHT> buffers = {};
    } m_Command = {};

    struct {
        std::array<vk::Semaphore, detail::MAX_FRAMES_IN_FLIGHT> presentComplete = {};
        std::vector<vk::Semaphore> renderFinished = {};
    } m_Semaphores = {};

    struct {
        std::array<vk::Fence, detail::MAX_FRAMES_IN_FLIGHT> inFlight = {};
        vk::Fence immediate = nullptr;
    } m_Fences = {};
};

} // namespace Physbuzz
