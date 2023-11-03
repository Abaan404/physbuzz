#include "ui.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

UserInferface::UserInferface(Painter &painter) : painter(painter) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void UserInferface::draw() {
    begin_frame();

    ImGui::ShowDemoWindow((bool *)true);

    end_frame();
}
