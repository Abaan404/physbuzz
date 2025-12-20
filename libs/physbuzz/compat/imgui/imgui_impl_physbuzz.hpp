#pragma once

#if !defined(IMGUI_DISABLE)

#include "../../ecs/system.hpp"
#include "../../render/renderers/defines.hpp"
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
    void resize(const glm::uvec2 &resolution);

    vk::DescriptorPool m_Pool = nullptr;
    glm::uvec2 m_Resolution;

    struct {
        EventID resize = -1;
    } m_Events = {};
};

} // namespace Physbuzz

#endif
