#pragma once

#if !defined(IMGUI_DISABLE)

#include "../../ecs/system.hpp"
#include "../../graphics/rendergraph.hpp"
#include <imgui.h>
#include <vulkan/vulkan.h>

namespace Physbuzz {

class Window;

class ImGuiRenderer : public System<> {
  public:
    struct Info {
        std::shared_ptr<Window> window;
    };

    ImGuiRenderer(const Info &info);

    bool build() override;
    bool destroy() override;

    void newFrame();

    const RenderGraph &getGraph() const;

  private:
    Info m_Info;

    vk::DescriptorPool m_Pool = nullptr;

    RenderGraph m_Graph;
};

} // namespace Physbuzz

#endif
