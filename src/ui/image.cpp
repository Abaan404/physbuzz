#include "image.hpp"
#include "ui.hpp"

#include <imgui.h>
#include <physbuzz/compat/imgui/imgui_impl_physbuzz.hpp>
#include <physbuzz/graphics/descriptors/attachment.hpp>
#include <physbuzz/graphics/descriptors/texture.hpp>
#include <physbuzz/graphics/renderer.hpp>

template <typename T>
Image<T>::Image(const Physbuzz::Resource<T> &attachment, Physbuzz::Scene *scene)
    : IUserInterface(scene), m_Resource(attachment) {}

template <typename T>
void Image<T>::draw() {
    std::shared_ptr<Physbuzz::Renderer> renderer = m_Scene->getSystem<Physbuzz::Renderer>();

    ImVec2 resolution = getResolution();
    float aspectRatio = resolution.x / resolution.y;

    float data[] = {aspectRatio, m_WidgetHeight};

    ImGuiSizeCallback callback = [](ImGuiSizeCallbackData *data) {
        // https://github.com/ocornut/imgui/pull/8028
        float aspectRatio = reinterpret_cast<float *>(data->UserData)[0];
        float widgetHeight = reinterpret_cast<float *>(data->UserData)[1];

        data->DesiredSize.y = data->DesiredSize.x / aspectRatio + widgetHeight;

        switch (ImGui::GetMouseCursor()) {
        case ImGuiMouseCursor_ResizeNWSE:
        case ImGuiMouseCursor_ResizeNESW:
            if (aspectRatio > data->DesiredSize.x / (data->DesiredSize.y - widgetHeight)) {
                data->DesiredSize.x = aspectRatio * (data->DesiredSize.y - widgetHeight);
            } else {
                data->DesiredSize.y = data->DesiredSize.x / aspectRatio + widgetHeight;
            }
            break;

        case ImGuiMouseCursor_ResizeNS:
            data->DesiredSize.x = aspectRatio * (data->DesiredSize.y - widgetHeight);
            break;
        case ImGuiMouseCursor_ResizeEW:
            data->DesiredSize.y = data->DesiredSize.x / aspectRatio + widgetHeight;
            break;
        }
    };

    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(resolution.x, resolution.y + m_WidgetHeight), callback, (void *)&data);
    ImGui::SetNextWindowSize(ImVec2(640, 360));

    std::string label = std::format("Image '{}'", m_Resource);

    if (!ImGui::Begin(label.c_str(), &show)) {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Aspect Mask");

    const std::vector<std::pair<std::string, Physbuzz::Image::AspectFlags>> aspectMasks = {
        {"Color", Physbuzz::Image::AspectFlags::eColor},
        {"Depth", Physbuzz::Image::AspectFlags::eDepth},
        {"Stencil", Physbuzz::Image::AspectFlags::eStencil},
    };

    ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_NoAutoSelect;
    ImGuiMultiSelectIO *msIO = ImGui::BeginMultiSelect(flags, -1, aspectMasks.size());

    for (std::size_t i = 0; i < aspectMasks.size(); i++) {
        const auto &[name, flag] = aspectMasks[i];

        bool isSelected = (m_ViewInfo.subresourceRange.aspectMask & flag) != Physbuzz::Image::AspectFlags::eNone;

        ImGui::SetNextItemSelectionUserData(i);
        if (ImGui::Selectable(name.c_str(), isSelected)) {
            if (isSelected) {
                m_ViewInfo.subresourceRange.aspectMask &= ~flag;
            } else {
                m_ViewInfo.subresourceRange.aspectMask |= flag;
            }
        }
    }

    msIO = ImGui::EndMultiSelect();

    std::uint32_t step = 1;

    ImGui::SeparatorText("Mip Level");

    ImGui::PushID("miplevel");
    ImGui::InputScalar("base", ImGuiDataType_U32, &m_ViewInfo.subresourceRange.baseMipLevel, &step);
    ImGui::InputScalar("count", ImGuiDataType_U32, &m_ViewInfo.subresourceRange.levelCount, &step);
    ImGui::PopID();

    ImGui::SeparatorText("Array Layer");

    ImGui::PushID("arraylayer");
    ImGui::InputScalar("base", ImGuiDataType_U32, &m_ViewInfo.subresourceRange.baseArrayLayer, &step);
    ImGui::InputScalar("count", ImGuiDataType_U32, &m_ViewInfo.subresourceRange.layerCount, &step);
    ImGui::PopID();

    std::shared_ptr<Physbuzz::ImGuiRenderer> imguiImpl = m_Scene->getSystem<Physbuzz::ImGuiRenderer>();
    std::optional<ImTextureID> id = imguiImpl->getTexture(m_Resource, m_ViewInfo);

    // use this to compute constraints next frame
    m_WidgetHeight = ImGui::GetWindowHeight() - ImGui::GetContentRegionAvail().y;

    ImGui::SeparatorText("Image");

    if (id) {
        ImGui::Image(*id, ImGui::GetContentRegionAvail());
    } else {
        ImGui::Text("No view found...");
    }

    ImGui::End();
}

template <>
ImVec2 Image<Physbuzz::Texture>::getResolution() {
    glm::uvec3 resolution = m_Resource->getSize();
    return ImVec2(resolution.x, resolution.y);
}

template <>
ImVec2 Image<Physbuzz::Attachment>::getResolution() {
    std::shared_ptr<Physbuzz::ImGuiRenderer> imguiImpl = m_Scene->getSystem<Physbuzz::ImGuiRenderer>();

    glm::uvec2 resolution = m_Resource->getSize(imguiImpl->getFrameInFlight());
    return ImVec2(resolution.x, resolution.y);
}

template class Image<Physbuzz::Texture>;
template class Image<Physbuzz::Attachment>;
