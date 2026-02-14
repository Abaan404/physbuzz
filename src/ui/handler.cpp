#include "handler.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <memory>
#include <physbuzz/compat/imgui/imgui_impl_physbuzz.hpp>

#include "camera.hpp"
#include "demo.hpp"
#include "dockspace.hpp"
#include "objectlist.hpp"
#include "objectpicker.hpp"
#include "overlay.hpp"
#include "rendergraph.hpp"

InterfaceManager::InterfaceManager() {}

bool InterfaceManager::build() {
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows (buggy on wayland)
    io.IniFilename = nullptr; // disable imgui.ini

    m_Interfaces.emplace_back(std::make_unique<Dockspace>(m_Scene));

    std::shared_ptr<IUserInterface> demo = m_Interfaces.emplace_back(std::make_shared<Demo>(m_Scene));
    // m_Interfaces["ObjectPicker"] = std::make_unique<ObjectPicker>(m_Scene);
    m_Interfaces.emplace_back(std::make_unique<ObjectList>(m_Scene));
    m_Interfaces.emplace_back(std::make_unique<Camera>(m_Scene));
    m_Interfaces.emplace_back(std::make_unique<RenderGraph>(m_Scene));

    demo->show = false;

    return true;
}

bool InterfaceManager::destroy() {
    return true;
}

void InterfaceManager::tick() {
    m_Scene->getSystem<Physbuzz::ImGuiRenderer>()->newFrame();
    ImGui::NewFrame();

    static FrametimeOverlay frametimeOverlay = {m_Scene};
    frametimeOverlay.draw();

    if (draw) {
        for (const auto &interface : m_Interfaces) {
            if (interface->show) {
                interface->draw();
            }
        }
    }

    ImGui::Render();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
