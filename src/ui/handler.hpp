#pragma once

#include "../renderer/renderer.hpp"
#include "ui.hpp"
#include <imgui.h>
#include <unordered_map>

enum class InterfaceType;

class InterfaceHandler {
  public:
    InterfaceHandler(Renderer &renderer, std::vector<std::shared_ptr<GameObject>> &objects);
    ~InterfaceHandler();

    bool draw = false;
    ImGuiIO &io = ImGui::GetIO();
    std::unordered_map<InterfaceType, std::unique_ptr<IUserInterface>> interfaces;

    void render();

  private:
    Renderer &renderer;
    std::vector<std::shared_ptr<GameObject>> &objects;
};
