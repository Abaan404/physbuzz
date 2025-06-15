#include "overlay.hpp"

#include "../../game.hpp"
#include <imgui.h>
#include <physbuzz/misc/clock.hpp>
#include <physbuzz/misc/context.hpp>

void FrametimeOverlay::draw() {
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

    Game *game = Physbuzz::Context::get<Game>();

    if (ImGui::Begin("Frametime", &show, window_flags)) {
        ImGui::Text("Frametime");
        ImGui::Separator();

        float duration = game->scene.getSystem<Physbuzz::Clock>()->getDelta();
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
