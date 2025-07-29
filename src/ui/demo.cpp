#include "demo.hpp"

#include <imgui.h>

Demo::Demo(Physbuzz::Scene *scene)
    : IUserInterface(scene) {}

void Demo::draw() {
    ImGui::ShowDemoWindow(&show);
}
