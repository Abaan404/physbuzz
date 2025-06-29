#include "camera.hpp"

#include "../../game.hpp"
#include "../../objects/common.hpp"
#include "../../objects/player.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/gl/capabilities.hpp>

constexpr float MAX_VALUE = 1000.0f;
constexpr float MIN_VALUE = -1000.0f;

void Camera::draw() {
    const ImGuiViewport *Viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(Viewport->WorkPos.x, Viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(128, 256), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = 0;
    if (!ImGui::Begin("Camera", &show, windowFlags)) {
        ImGui::End();
        return;
    }

    Game *game = Physbuzz::Context::get<Game>();

    for (const auto &[player, camera, identifier] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, IdentifiableComponent>()) {
        ImGui::SeparatorText(identifier.name.c_str());

        ImGui::SeparatorText("Projection");

        Physbuzz::CameraComponent::Info info = camera.getInfo();

        const char *projections[] = {"Prespective", "Orthographic", "Unknown"};
        static int currentProjection = static_cast<int>(info.projection);

        if (ImGui::Combo("projection", &currentProjection, projections, IM_ARRAYSIZE(projections))) {
            glm::vec2 resolution = game->window.getResolution();
            switch (currentProjection) {
            case 0:
                info.projection = Physbuzz::CameraComponent::Projection::Perspective;
                info.perspective = {
                    .fovy = glm::radians(45.0f),
                    .aspect = resolution.x / resolution.y,
                };
                break;

            case 1: // Orthographic
                info.projection = Physbuzz::CameraComponent::Projection::Orthographic;
                info.orthographic = {
                    .left = 0.0f,
                    .right = resolution.x,
                    .bottom = resolution.y,
                    .top = 0.0f,
                };
                break;

            default:
                break;
            }

            camera.update(info);
            camera.reset();
        }

        switch (currentProjection) {
        // Prespective
        case 0: {
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

        // Orthographic
        case 1: {
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
    }

    ImGui::End();
}
