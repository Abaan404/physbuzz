#pragma once

#include "../renderer/renderer.hpp"
#include "ui.hpp"
#include <imgui.h>
#include <unordered_map>

class InterfaceHandler {
  public:
    InterfaceHandler(Renderer &renderer, Scene &scene);
    ~InterfaceHandler();

    ImGuiIO &io = ImGui::GetIO();
    ImGuiViewport *viewport = ImGui::GetMainViewport();

    bool draw = false;
    std::unordered_map<std::string, std::unique_ptr<IUserInterface>> interfaces;

    void render();

  private:
    Renderer &renderer;
    Scene &scene;
};
