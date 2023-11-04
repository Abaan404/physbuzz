#pragma once

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "painter.hpp"

enum class InterfaceType {
    Debug,
    Info,
    MaxValue // not an actual type, just used to loop back to idx 0
};

class UserInferface {
  public:
    UserInferface(Painter &painter) : painter(painter) {}

    void render();

    ImGuiIO& io = ImGui::GetIO();

    InterfaceType interface_type;
    bool draw_interface = false;

  private:
    bool draw_demo_window = true;
    Painter &painter;
};
