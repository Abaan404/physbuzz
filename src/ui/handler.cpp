#include "handler.hpp"

#include "../renderer/renderer.hpp"
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <memory>

#include "demo/demo.hpp"

enum class InterfaceType {
    Debug,
    ShapePicker,
    ObjectList,
    SceneViewer,
    Demo,
    Unknown,
};

InterfaceHandler::InterfaceHandler(Renderer &renderer, std::vector<std::shared_ptr<GameObject>> &objects) : renderer(renderer), objects(objects) {
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    ImGui_ImplSDL2_InitForOpenGL(renderer.window, renderer.context);
    ImGui_ImplOpenGL3_Init("#version 460");

    interfaces[InterfaceType::Demo] = std::make_unique<Demo>();

    interfaces[InterfaceType::Demo]->show = true;
}

InterfaceHandler::~InterfaceHandler() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void InterfaceHandler::render() {
    if (!draw)
        return;

    // draw a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    for (auto const &interface : interfaces) {
        if (interface.second->show)
            interface.second->draw(renderer);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
