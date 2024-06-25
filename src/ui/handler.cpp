#include "handler.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>

#include "demo/demo.hpp"
#include "dockspace/dockspace.hpp"
#include "objectlist/objectlist.hpp"
#include "objectpicker/objectpicker.hpp"
#include "overlay/overlay.hpp"

InterfaceManager::InterfaceManager(Physbuzz::Renderer &renderer)
    : m_Renderer(renderer) {}

InterfaceManager::~InterfaceManager() {}

void InterfaceManager::build() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows (buggy on wayland)
    io.IniFilename = nullptr; // disable imgui.ini

    GLFWwindow *window = m_Renderer.getWindow().getWindow();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    interfaces["Demo"] = std::make_shared<Demo>();
    interfaces["ShapePicker"] = std::make_unique<ObjectPicker>();
    interfaces["ObjectList"] = std::make_unique<ObjectList>();
    interfaces["Dockspace"] = std::make_unique<Dockspace>();

    interfaces["Demo"]->show = false;
}

void InterfaceManager::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

InterfaceManager::InterfaceManager(const InterfaceManager &other) : m_Renderer(other.m_Renderer) {
    if (this != &other) {
        interfaces = other.interfaces;
        draw = other.draw;
    }
}

InterfaceManager InterfaceManager::operator=(const InterfaceManager &other) {
    if (this != &other) {
        interfaces = other.interfaces;
        draw = other.draw;

        m_Renderer = other.m_Renderer;
    }

    return *this;
}

void InterfaceManager::render() {
    // draw a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static FrametimeOverlay frametimeOverlay;
    frametimeOverlay.draw(m_Renderer);

    if (draw) {
        for (const auto &interface : interfaces) {
            if (interface.second->show) {
                interface.second->draw(m_Renderer);
            }
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
