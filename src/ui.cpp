#include "ui.hpp"

void UserInferface::render() {
    // draw a new frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (!draw_interface)
        return;

    // show a demo window
    if (draw_demo_window)
        ImGui::ShowDemoWindow(&draw_demo_window);

    // ImGui_ImplSDLRenderer2_RenderDrawData() gets
    // called from Painter::clear()
}
