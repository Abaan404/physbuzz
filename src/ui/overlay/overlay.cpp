#include "overlay.hpp"

#include "../../game.hpp"
#include <imgui.h>
#include <physbuzz/context.hpp>

void FrametimeOverlay::draw(Physbuzz::Renderer &renderer) {
    Game *game = Physbuzz::Context::get<Game>(renderer.getWindow().getGLFWwindow());

    ImGuiIO &io = ImGui::GetIO();
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos({10.0f, 10.0f}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    if (ImGui::Begin("Frametime", &show, window_flags)) {
        ImGui::Text("Frametime");
        ImGui::Separator();

        float duration = game->frameClock.getDelta().count();
        ImGui::Text("FPS: %.2f (%.2f ms)", 1000.0f / duration, duration);

        if (ImGui::BeginPopupContextWindow()) {
            if (show && ImGui::MenuItem("Close")) {
                show = false;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}
