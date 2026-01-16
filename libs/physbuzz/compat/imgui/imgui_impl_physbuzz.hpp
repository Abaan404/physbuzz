#pragma once

#if !defined(IMGUI_DISABLE)

#include "../../ecs/system.hpp"
#include "../../graphics/defines.hpp"
#include <imgui.h>
#include <vulkan/vulkan.h>

namespace Physbuzz {

class ImGuiRenderer : public IRenderPass,
                      public System<> {
  public:
    ImGuiRenderer();

    bool build() override;
    bool destroy() override;

    void render(const RenderContext &context) override;
    void newFrame();

  private:
    vk::DescriptorPool m_Pool = nullptr;
};

} // namespace Physbuzz

#endif
