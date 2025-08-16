#pragma once

#include "../model.hpp"
#include "../transform.hpp"
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Framebuffer;

struct RenderCommand {
    std::uint32_t frameInFlight = 0;
    std::uint32_t maxFramesInFlight = 2;

    vk::CommandPool pool = nullptr;
    std::vector<vk::CommandBuffer> buffers = {};
};

class IRenderer {
  public:
    virtual ~IRenderer() = default;

    IRenderer(const RenderCommand &command)
        : m_Command(&command) {}

    virtual void resize(const glm::ivec2 &resolution) = 0;
    virtual const Framebuffer &getOutput() const = 0;

  protected:
    const RenderCommand *m_Command;
};

struct RenderComponent {
    Transform transform;
    Resource<Model> model;
};

} // namespace Physbuzz
