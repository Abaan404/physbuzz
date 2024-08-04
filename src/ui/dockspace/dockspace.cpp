#include "dockspace.hpp"

#include <imgui.h>
#include <imgui_internal.h> // for DockBuilder API

void Dockspace::draw() {
    // setup dockspace
    ImGuiDockNodeFlags dockspaceFlags = 0;
    ImGuiWindowFlags windowFlags = (ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

    ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    // yeet style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    // dockspace
    ImGuiID dockspace = viewport->ID;
    ImGui::DockSpace(dockspace, ImVec2(0.0f, 0.0f), dockspaceFlags | ImGuiDockNodeFlags_PassthruCentralNode);

    static auto runOnce = [&]() {
        // DockBuilder is experimental
        // see: https://github.com/ocornut/imgui/wiki/Docking
        ImGui::DockBuilderAddNode(dockspace, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace, viewport->Size);

        // split dockspace
        ImGuiID dockspaceRight = ImGui::GetID("DockspaceRight");
        ImGuiID dockspaceLeftMiddle = ImGui::GetID("DockspaceLeftMiddle");
        ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.2f, &dockspaceRight, &dockspaceLeftMiddle);

        // setup right hand side
        {
            ImGuiID dockspaceObjectsList = ImGui::GetID("DockspaceObjectsList");
            ImGuiID dockspaceObjectsPicker = ImGui::GetID("DockspaceObjectsPicker");
            ImGui::DockBuilderSplitNode(dockspaceRight, ImGuiDir_Up, 0.5f, &dockspaceObjectsList, &dockspaceObjectsPicker);
            ImGui::DockBuilderDockWindow("ObjectList", dockspaceObjectsList);
            ImGui::DockBuilderDockWindow("ShapePicker", dockspaceObjectsPicker);
        }

        ImGuiID dockspaceLeft = ImGui::GetID("DockspaceLeft");
        ImGuiID dockspaceMiddle = ImGui::GetID("DockspaceMiddle");
        ImGui::DockBuilderSplitNode(dockspaceLeftMiddle, ImGuiDir_Left, 0.25f, &dockspaceLeft, &dockspaceMiddle);

        // setup middle
        {
            // TODO viewport framebuffer?
        }

        // setup left hand side
        {
            ImGuiID dockspaceCamera = ImGui::GetID("DockspaceLeft");
            ImGui::DockBuilderDockWindow("Camera", dockspaceLeft);
        }

        ImGui::DockBuilderFinish(dockspace);

        return 0;
    }();

    ImGui::End();
}
