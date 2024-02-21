#include "viewport.hpp"
#include <imgui.h>

Viewport::Viewport(Renderer &renderer) : renderer(renderer) {}

void Viewport::draw(Renderer &renderer) {
    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags window_flags = 0;
    if (!ImGui::Begin("Viewport", &show, window_flags)) {
        ImGui::End();
        return;
    }

    ImVec2 wsize = ImGui::GetWindowSize();

    renderer.target(&framebuffer);
    renderer.clear(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    renderer.resize(glm::ivec2(wsize.x, wsize.y));
    renderer.render();
    renderer.target(nullptr);

    ImGui::Image((ImTextureID)(intptr_t)framebuffer.get_color_id(), wsize, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}
