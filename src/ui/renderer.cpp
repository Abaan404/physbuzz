#include "renderer.hpp"

#include <format>
#include <imgui.h>
#include <physbuzz/graphics/renderer.hpp>

static void drawImageWindow(std::string label, bool *show, ImTextureID id, const ImVec2 &resolution) {
    float aspect_ratio = resolution.x / resolution.y;

    ImGui::SetNextWindowSizeConstraints(
        ImVec2(427, 240),
        ImVec2(resolution.x, resolution.y),
        [](ImGuiSizeCallbackData *data) {
            // https://github.com/ocornut/imgui/pull/8028
            float aspect_ratio = *(float *)data->UserData;
            data->DesiredSize.y = data->DesiredSize.x / aspect_ratio;

            switch (ImGui::GetMouseCursor()) {
            case ImGuiMouseCursor_ResizeNWSE:
            case ImGuiMouseCursor_ResizeNESW:
                if (aspect_ratio > data->DesiredSize.x / data->DesiredSize.y) {
                    data->DesiredSize.x = aspect_ratio * data->DesiredSize.y;
                } else {
                    data->DesiredSize.y = data->DesiredSize.x / aspect_ratio;
                }
                break;

            case ImGuiMouseCursor_ResizeNS:
                data->DesiredSize.x = aspect_ratio * data->DesiredSize.y;
                break;
            case ImGuiMouseCursor_ResizeEW:
                data->DesiredSize.y = data->DesiredSize.x / aspect_ratio;
                break;
            }
        },
        (void *)&aspect_ratio);

    if (!ImGui::Begin(label.c_str(), show)) {
        ImGui::End();
        return;
    }

    ImGui::Image(id, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

Renderer::Renderer(Physbuzz::Scene *scene)
    : IUserInterface(scene) {}

void Renderer::draw() {
    const std::shared_ptr<Physbuzz::Renderer> renderer = m_Scene->getSystem<Physbuzz::Renderer>();

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("Renderer", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    // const char *types[] = {"Deferred", "Forward"};
    // static int currentType = static_cast<int>(renderer->getInfo().type);
    //
    // if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types))) {
    //     switch (currentType) {
    //     case 0: // Deferred
    //         renderer->setType(Physbuzz::Renderer::Type::Deferred);
    //         break;
    //
    //     case 1: // Forward
    //         renderer->setType(Physbuzz::Renderer::Type::Forward);
    //         break;
    //
    //     default:
    //         break;
    //     }
    // }
    //
    // if (renderer->getInfo().type == Physbuzz::Renderer::Type::Deferred) {
    //     const std::shared_ptr<Physbuzz::DeferredRenderer> deferred = m_Scene->getSystem<Physbuzz::DeferredRenderer>();
    //     const Physbuzz::DeferredRenderer::Framebuffers &framebuffers = deferred->getFramebuffers();
    //
    //     const Physbuzz::Framebuffer::Info &gBufferInfo = framebuffers.gBuffer.getInfo();
    //
    //     static bool showWindow = false;
    //     static std::size_t selectedColorIndex;
    //
    //     if (ImGui::Button("GBuffers")) {
    //         ImGui::OpenPopup("popup-gbuffer");
    //     }
    //
    //     if (ImGui::BeginPopup("popup-gbuffer")) {
    //         ImGui::SeparatorText("Select GBuffer");
    //
    //         for (std::size_t i = 0; i < gBufferInfo.colors.size(); i++) {
    //             if (gBufferInfo.colors[i].isDrawn && gBufferInfo.colors[i].storage == Physbuzz::Framebuffer::Storage::Texture2D) {
    //                 if (ImGui::Selectable(std::format("Buffer {}", i).c_str())) {
    //                     selectedColorIndex = i;
    //                     showWindow = true;
    //                 }
    //             }
    //         }
    //
    //         ImGui::EndPopup();
    //     }
    //
    //     if (showWindow && selectedColorIndex < gBufferInfo.colors.size()) {
    //         drawImageWindow(
    //             "GBuffer",
    //             &showWindow,
    //             (ImTextureID)(intptr_t)framebuffers.gBuffer.getImGuiTextureHandle(Physbuzz::Framebuffer::Type::Color, selectedColorIndex),
    //             {static_cast<float>(gBufferInfo.resolution.x), static_cast<float>(gBufferInfo.resolution.y)});
    //     }
    // }

    ImGui::End();
}
