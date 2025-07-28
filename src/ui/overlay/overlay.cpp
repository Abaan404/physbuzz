#include "overlay.hpp"

#include <imgui.h>
#include <physbuzz/misc/clock.hpp>

FrametimeOverlay::FrametimeOverlay(Physbuzz::Scene *scene)
    : IUserInterface(scene) {}

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

    if (ImGui::Begin("Frametime", &show, window_flags)) {
        ImGui::Text("Frametime");
        ImGui::Separator();

        float duration = m_Scene->getSystem<Physbuzz::Clock>()->getDelta();
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
