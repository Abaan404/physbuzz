#include "ui.hpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

UserInferface::UserInferface(Renderer &renderer) : renderer(renderer) {
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Does not work under wayland, use xwayland to support this
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(renderer.window, renderer.context);
    ImGui_ImplOpenGL3_Init("#version 460");
}

UserInferface::~UserInferface() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void UserInferface::show_shape_picker() {
    // const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    // ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    // ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);
    //
    // ImGuiWindowFlags window_flags = 0;
    // ImGui::Begin("Shape Picker", &draw_shape_picker, window_flags);
    //
    // ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
    //
    // ImGui::Text("dear imgui says hello! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    // ImGui::Spacing();
    //
    // static const int image_size = 40;
    //
    // static std::shared_ptr<Box> box = std::make_shared<Box>(glm::vec2(image_size >> 1, image_size >> 1), image_size, image_size);
    // static std::shared_ptr<Circle> circle = std::make_shared<Circle>(glm::vec2(image_size >> 1, image_size >> 1), image_size);
    //
    // static SDL_Color box_color = {255, 255, 255, 255};
    // static SDL_Color physics_box_color = {255, 255, 255, 255};
    // static SDL_Color circle_color = {255, 255, 255, 255};
    // static SDL_Color physics_circle_color = {255, 255, 255, 255};
    //
    // std::unordered_map<std::string, SDL_Texture *> textures = {
    //     {"box", renderer.draw_box(box, box_color)},
    //     {"circle", renderer.draw_circle(circle, circle_color)},
    //     {"physics_box", renderer.draw_box(box, physics_box_color)},
    //     {"physics_circle", renderer.draw_circle(circle, physics_circle_color)},
    // };
    //
    // static int pressed_count = 0;
    //
    // int i = 0;
    // for (auto &pair : textures) {
    //     // UV coordinates are often (0.0f, 0.0f) and (1.0f, 1.0f) to display an entire textures.
    //     // Here are trying to display only a 32x32 pixels area of the texture, hence the UV computation.
    //     // Read about UV coordinates here: https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
    //     ImGui::PushID(i);
    //     // if (i > 0)
    //     //     ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(i - 1.0f, i - 1.0f));
    //     ++i;
    //
    //     ImGui::Image((void *)pair.second, {image_size, image_size});
    //
    //     // ImVec2 size = ImVec2(32.0f, 32.0f);                      // Size of the image we want to make visible
    //     // ImVec2 uv0 = ImVec2(0.0f, 0.0f);                         // UV coordinates for lower-left
    //     // ImVec2 uv1 = ImVec2(32.0f / my_tex_w, 32.0f / my_tex_h); // UV coordinates for (32,32) in our texture
    //     // ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);          // Black background
    //     // ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // No tint
    //
    //     // if (ImGui::ImageButton("", my_tex_id, size, uv0, uv1, bg_col, tint_col))
    //     //     pressed_count += 1;
    //     if (i > 0)
    //         ImGui::PopStyleVar();
    //
    //     ImGui::PopID();
    //     ImGui::SameLine();
    // }
    //
    // ImGui::End();
}

void UserInferface::render() {
    if (!draw_interface)
        return;

    // draw a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // show a demo window
    if (draw_demo_window)
        ImGui::ShowDemoWindow(&draw_demo_window);

    // if (draw_shape_picker)
    //     this->show_shape_picker();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
