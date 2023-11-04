#include "ui.hpp"

void UserInferface::render() {
    // draw a new frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // show a demo window
    ImGui::ShowDemoWindow((bool *)true);

    // ImGui_ImplSDLRenderer2_RenderDrawData() gets
    // called from Painter::clear()
}
