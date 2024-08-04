#include "handler.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>

#include "camera/camera.hpp"
#include "demo/demo.hpp"
#include "dockspace/dockspace.hpp"
#include "objectlist/objectlist.hpp"
#include "objectpicker/objectpicker.hpp"
#include "overlay/overlay.hpp"

InterfaceManager::InterfaceManager() {}

InterfaceManager::~InterfaceManager() {}

void InterfaceManager::build(const Physbuzz::Window &window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows (buggy on wayland)
    io.IniFilename = nullptr; // disable imgui.ini

    ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");

    m_Interfaces["Demo"] = std::make_shared<Demo>();
    m_Interfaces["ShapePicker"] = std::make_unique<ObjectPicker>();
    m_Interfaces["ObjectList"] = std::make_unique<ObjectList>();
    m_Interfaces["Camera"] = std::make_unique<Camera>();
    m_Interfaces["Dockspace"] = std::make_unique<Dockspace>();

    m_Interfaces["Demo"]->show = false;
}

void InterfaceManager::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void InterfaceManager::render() {
    // draw a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static FrametimeOverlay frametimeOverlay;
    frametimeOverlay.draw();

    if (draw) {
        for (const auto &interface : m_Interfaces) {
            if (interface.second->show) {
                interface.second->draw();
            }
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
