// #include "objectlist.hpp"
//
// #include <imgui.h>
//
// ObjectList::ObjectList(std::vector<std::shared_ptr<GameObject>> &objects) : objects(objects) {}
//
// void ObjectList::object_box(Box &box, Renderer &renderer) {
//     ImGui::SeparatorText("Box");
//
//     ImGui::Text("Position");
//     {
//         float position[] = {box.position.x, box.position.y};
//         ImGui::DragFloat2("x", position, 0.01f, 0.0f, 1.0f);
//         box.position = glm::vec2(box.position.x, box.position.y);
//     }
// }
//
// void ObjectList::object_circle(Circle &circle, Renderer &renderer) {
//     ImGui::SeparatorText("Circle");
//
//     {
//         float position[] = {circle.position.x, circle.position.y};
//         ImGui::DragFloat2("position", position, 1.00f, 0.0f, 1000.0f);
//         circle.position = glm::vec2(position[0], position[1]);
//     }
//
//     ImGui::DragFloat("radius", &circle.radius, 1.00f, 0.0f, 1000.0f);
// }
//
// // TODO make this useable
//
// void ObjectList::draw(Renderer &renderer) {
//     const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
//     ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
//     ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);
//
//     ImGuiWindowFlags window_flags = 0;
//     if (!ImGui::Begin("ObjectList", &show, window_flags)) {
//         ImGui::End();
//         return;
//     }
//
//     ImGui::Text("Spawned Objects: %zu", objects.size());
//
//     ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//     if (ImGui::TreeNode("Objects")) {
//         for (int i = 0; i < objects.size(); i++) {
//             ImGui::PushID(i);
//             switch (objects[i]->identifier) {
//             case Objects::Box:
//                 object_box(*std::static_pointer_cast<Box>(objects[i]), renderer);
//                 break;
//
//             case Objects::Circle:
//                 object_circle(*std::static_pointer_cast<Circle>(objects[i]), renderer);
//                 break;
//
//             default:
//                 break;
//             }
//             ImGui::PopID();
//         }
//         ImGui::TreePop();
//     }
//     ImGui::End();
// }
