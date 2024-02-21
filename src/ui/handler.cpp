#include "handler.hpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <memory>

#include "demo/demo.hpp"
#include "dockspace/dockspace.hpp"
#include "objectlist/objectlist.hpp"
#include "objectpicker/objectpicker.hpp"
#include "viewport/viewport.hpp"

InterfaceHandler::InterfaceHandler(Renderer &renderer, std::vector<std::shared_ptr<GameObject>> &objects) : renderer(renderer), objects(objects) {
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    io.IniFilename = nullptr;

    ImGui_ImplSDL2_InitForOpenGL(renderer.window, renderer.context);
    ImGui_ImplOpenGL3_Init("#version 460");

    interfaces["ShapePicker"] = std::make_unique<ObjectPicker>();
    interfaces["Demo"] = std::make_unique<Demo>();
    interfaces["ObjectList"] = std::make_unique<ObjectList>(objects);
    interfaces["Viewport"] = std::make_unique<Viewport>(renderer);
    interfaces["Dockspace"] = std::make_unique<Dockspace>();

    interfaces["Demo"]->show = false;
}

InterfaceHandler::~InterfaceHandler() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void InterfaceHandler::render() {
    // draw a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (draw)
        for (const auto &interface : interfaces)
            if (interface.second->show)
                interface.second->draw(renderer);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
