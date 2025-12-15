#pragma once

#if !defined(IMGUI_DISABLE)

#include "../../ecs/system.hpp"
#include "../../render/renderers/defines.hpp"
#include "../../window/window.hpp"
#include <imgui.h>
#include <vulkan/vulkan.h>

namespace Physbuzz {

class ImGuiRenderer : public IRenderPass,
                      public System<> {
  public:
    struct Info {
        const std::shared_ptr<Window> window;
    };

    ImGuiRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void render(const vk::CommandBuffer &commandBuffer, std::uint32_t frameInFlight) override;
    void newFrame();

  private:
    Info m_Info;

    vk::DescriptorPool m_Pool = nullptr;
};

} // namespace Physbuzz

#endif
