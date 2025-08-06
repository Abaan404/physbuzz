#include "handler.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <memory>

#include "camera.hpp"
#include "demo.hpp"
#include "dockspace.hpp"
#include "objectlist.hpp"
#include "objectpicker.hpp"
#include "overlay.hpp"
#include "renderer.hpp"

InterfaceManager::InterfaceManager(const Info &info)
    : m_Info(info) {}

bool InterfaceManager::build() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows (buggy on wayland)
    io.IniFilename = nullptr; // disable imgui.ini

    ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow *>(*m_Info.window), true);
    // ImGui_ImplVulkan_Init();

    m_Interfaces["Demo"] = std::make_shared<Demo>(m_Scene);
    // m_Interfaces["ShapePicker"] = std::make_unique<ObjectPicker>(m_Scene);
    m_Interfaces["ObjectList"] = std::make_unique<ObjectList>(m_Scene);
    m_Interfaces["Camera"] = std::make_unique<Camera>(m_Scene);
    m_Interfaces["Renderer"] = std::make_unique<Renderer>(m_Scene);
    m_Interfaces["Dockspace"] = std::make_unique<Dockspace>(m_Scene);

    m_Interfaces["Demo"]->show = false;

    return true;
}

bool InterfaceManager::destroy() {
    // ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return true;
}

void InterfaceManager::tick() {
    // draw a new frame
    // ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static FrametimeOverlay frametimeOverlay = {m_Scene};
    frametimeOverlay.draw();

    if (draw) {
        for (const auto &interface : m_Interfaces) {
            if (interface.second->show) {
                interface.second->draw();
            }
        }
    }

    ImGui::Render();
    // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
