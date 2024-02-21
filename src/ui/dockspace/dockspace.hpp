#pragma once

#include "../ui.hpp"
#include <imgui.h>

class Dockspace : public IUserInterface {
  public:
    ImGuiViewport *viewport = ImGui::GetMainViewport();

    void draw(Renderer &renderer) override;
};
