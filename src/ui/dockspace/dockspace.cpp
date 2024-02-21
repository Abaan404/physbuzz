#include "dockspace.hpp"
#include <imgui.h>
#include <imgui_internal.h> // for DockBuilder API

void Dockspace::draw(Renderer &renderer) {
    // setup dockspace
    ImGuiDockNodeFlags dockspace_flags = 0;
    ImGuiWindowFlags window_flags = (ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    // yeet style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // dockspace
    ImGuiID dockspace = ImGui::GetMainViewport()->ID;
    ImGui::DockSpace(dockspace, ImVec2(0.0f, 0.0f), dockspace_flags | ImGuiDockNodeFlags_PassthruCentralNode);

    static bool first_time = true;
    if (first_time) {
        first_time = false;

        // DockBuilder is experimental
        // see: https://github.com/ocornut/imgui/wiki/Docking
        ImGui::DockBuilderAddNode(dockspace, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace, viewport->Size);

        // split dockspace
        ImGuiID dockspace_viewport = ImGui::GetID("DockspaceLeft");
        ImGuiID dockspace_objects = ImGui::GetID("DockspaceRight");
        ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.25f, &dockspace_objects, &dockspace_viewport);

        // setup right hand side
        {
            ImGuiID dockspace_objects_list = ImGui::GetID("DockspaceRightUp");
            ImGuiID dockspace_objects_picker = ImGui::GetID("DockspaceRightDown");
            ImGui::DockBuilderSplitNode(dockspace_objects, ImGuiDir_Up, 0.5f, &dockspace_objects_list, &dockspace_objects_picker);
            ImGui::DockBuilderDockWindow("ObjectList", dockspace_objects_list);
            ImGui::DockBuilderDockWindow("ShapePicker", dockspace_objects_picker);
        }

        // setup middle
        {
            ImGui::DockBuilderDockWindow("Viewport", dockspace_viewport);
        }

        ImGui::DockBuilderFinish(dockspace);
    }

    ImGui::End();
}
