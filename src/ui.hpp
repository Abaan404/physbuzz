#pragma once

#include "painter.hpp"

#include "imgui.h"

enum class InterfaceType {
    Debug,
    Info,
    MaxValue // not an actual type, just used to loop back to idx 0
};

class UserInferface {
  public:
    UserInferface(Painter &painter);
    ~UserInferface();

    void render();

    ImGuiIO &io = ImGui::GetIO();
    InterfaceType interface_type;

    bool draw_interface = false;
    bool draw_shape_picker = true;
    bool draw_demo_window = true;

  private:
    void show_shape_picker();

    Painter &painter;
};
