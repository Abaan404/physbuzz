// #include "objectpicker.hpp"
//
// #include "../../geometry/box/box.hpp"
// #include "../../geometry/circle/circle.hpp"
// #include <glad/gl.h>
// #include <glm/glm.hpp>
// #include <imgui.h>
// #include <memory>
//
// // TODO make this useable
//
// void ObjectPicker::draw(Renderer &renderer) {
//     const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
//     ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
//     ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);
//
//     ImGuiWindowFlags window_flags = 0;
//     if (!ImGui::Begin("ShapePicker", &show, window_flags)) {
//         ImGui::End();
//         return;
//     }
//
//     static glm::vec4 clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
//     static std::vector<std::shared_ptr<GameObject>> objects = {
//         std::make_shared<Box>(glm::vec2(preview_size.x >> 1, preview_size.y >> 1), preview_size.x, preview_size.y, 1.0f),
//         std::make_shared<Circle>(glm::vec2(preview_size.x >> 1, preview_size.y >> 1), glm::min(preview_size.x, preview_size.y) >> 1, 1.0f),
//     };
//
//     renderer.target(&framebuffer);
//     renderer.clear(clear_color);
//
//     objects[1]->texture->draw(renderer);
//
//     renderer.target(nullptr);
//
//     ImVec2 size = {(float)preview_size.x, (float)preview_size.y};
//     ImGui::Image((void *)(intptr_t)framebuffer.get_color_id(), size);
//
//     ImGui::End();
// }
