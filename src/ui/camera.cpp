#include "camera.hpp"

#include "../objects/player.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <physbuzz/graphics/renderer.hpp>

constexpr float MAX_VALUE = 1000.0f;
constexpr float MIN_VALUE = -1000.0f;

Camera::Camera(Physbuzz::Scene *scene)
    : IUserInterface(scene) {}

void Camera::draw() {
    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("Camera", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    std::shared_ptr<Physbuzz::Renderer> renderer = m_Scene->getSystem<Physbuzz::Renderer>();

    const auto [_, camera, player] = m_Scene->getComponents<Physbuzz::CameraComponent, PlayerComponent>().front();

    ImGui::SeparatorText("Projection");

    Physbuzz::CameraComponent::Info info = camera.getInfo();

    const std::array projections = {"Prespective", "Orthographic", "Unknown"};
    std::int32_t currentProjectionIdx = static_cast<int>(info.projection);

    m_SelectedProjection = projections[currentProjectionIdx];

    if (ImGui::BeginCombo("Type", m_SelectedProjection.c_str())) {
        for (std::size_t i = 0; i < projections.size(); i++) {
            std::string projection = projections[i];
            bool isSelected = i == currentProjectionIdx;

            if (ImGui::Selectable(projection.c_str(), isSelected)) {
                info.projection = static_cast<Physbuzz::CameraComponent::Projection>(i);
                glm::vec2 resolution = info.resolution;

                switch (info.projection) {
                case Physbuzz::CameraComponent::Projection::Perspective:
                    info.perspective = {
                        .fovy = glm::radians(45.0f),
                        .aspect = resolution.x / resolution.y,
                    };
                    break;

                case Physbuzz::CameraComponent::Projection::Orthographic:
                    info.orthographic = {
                        .left = 0.0f,
                        .right = resolution.x,
                        .bottom = resolution.y,
                        .top = 0.0f,
                    };
                    break;

                case Physbuzz::CameraComponent::Projection::Unknown:
                    break;
                }

                camera.update(info);
                camera.reset();
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    switch (info.projection) {
    case Physbuzz::CameraComponent::Projection::Perspective: {
        if (ImGui::DragFloat("fov", &info.perspective.fovy, 0.01f, 0.0f, 2.0f * glm::pi<float>())) {
            camera.update(info);
        }

        if (ImGui::DragFloat("aspect", &info.perspective.aspect, 0.1f, 0.0f, MAX_VALUE)) {
            camera.update(info);
        }

        Physbuzz::CameraComponent::Depth depth = info.depth;
        float depths[2] = {depth.near, depth.far};
        if (ImGui::DragFloat2("depth", depths, 1.0f, MIN_VALUE, MAX_VALUE)) {
            info.depth.near = depths[0];
            info.depth.far = depths[1];
            camera.update(info);
        }
    } break;

    case Physbuzz::CameraComponent::Projection::Orthographic: {
        Physbuzz::CameraComponent::Orthographic orthographic = info.orthographic;

        if (ImGui::DragFloat("top", &orthographic.top, 1.0f, MIN_VALUE, MAX_VALUE)) {
            camera.update(info);
        }

        if (ImGui::DragFloat("bottom", &orthographic.bottom, 1.0f, MIN_VALUE, MAX_VALUE)) {
            camera.update(info);
        }

        if (ImGui::DragFloat("left", &orthographic.left, 1.0f, MIN_VALUE, MAX_VALUE)) {
            camera.update(info);
        }

        if (ImGui::DragFloat("right", &orthographic.right, 1.0f, MIN_VALUE, MAX_VALUE)) {
            camera.update(info);
        }

        Physbuzz::CameraComponent::Depth depth = info.depth;
        float depths[2] = {depth.near, depth.far};
        if (ImGui::DragFloat2("depth", depths, 1.0f, MIN_VALUE, MAX_VALUE)) {
            info.depth.near = depths[0];
            info.depth.far = depths[1];
            camera.update(info);
        }

    } break;

    default:
        break;
    }

    ImGui::SeparatorText("View");

    if (ImGui::Button("Reset")) {
        camera.reset();
    }

    glm::vec3 position = info.view.position;
    if (ImGui::DragFloat3("position", glm::value_ptr(position), 1.0f, MIN_VALUE, MAX_VALUE)) {
        camera.setPosition(position);
    }

    glm::vec3 up = camera.getUp();
    if (ImGui::DragFloat3("up", glm::value_ptr(up), 0.01f, MIN_VALUE, MAX_VALUE)) {
        camera.setUp(glm::normalize(up));
    }

    glm::vec3 facing = camera.getFacing();
    if (ImGui::DragFloat3("facing", glm::value_ptr(facing), 0.01f, MIN_VALUE, MAX_VALUE)) {
        camera.setFacing(glm::normalize(facing));
    }

    ImGui::DragFloat("speed", &player.speed, 0.1f, 0.0f, MAX_VALUE);
    ImGui::DragFloat("senstivity", &player.sensitivity, 0.1f, 0.0f, MAX_VALUE);

    ImGui::End();
}
