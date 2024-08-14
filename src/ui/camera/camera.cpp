#include "camera.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "../../game.hpp"
#include <imgui.h>
#include <physbuzz/misc/context.hpp>

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

    ImGui::SeparatorText("Projection");
    const char *projections[] = {"Prespective", "Orthographic", "Orthographic2D", "Unknown"};
    static int currentProjection = static_cast<int>(game->player.camera.getProjectionType());

    if (ImGui::Combo("type", &currentProjection, projections, IM_ARRAYSIZE(projections))) {
        glm::vec2 resolution = game->window.getResolution();
        switch (currentProjection) {
        case 0:
            game->player.camera.setPrespective(
                {.fovy = glm::radians(45.0f), .aspect = resolution.x / resolution.y},
                {.near = 0.1f, .far = 1000.0f});
            break;

        case 1:
            game->player.camera.setOrthographic(
                {.left = 0.0f, .right = resolution.x, .bottom = resolution.y, .top = 0.0f},
                {.near = -1.0f, .far = 1.0f});
            break;

        case 2:
            game->player.camera.setOrthographic(resolution);
        default:
            break;
        }

        game->player.camera.resetView();
    }

    switch (currentProjection) {
    // Prespective
    case 0: {
        Physbuzz::Camera::Prespective prespective = game->player.camera.getPrespective();

        if (ImGui::DragFloat("fov", &prespective.fovy, 0.01f, 0.0f, 2.0f * glm::pi<float>())) {
            game->player.camera.setPrespective(prespective, game->player.camera.getDepth());
        }

        if (ImGui::DragFloat("aspect", &prespective.aspect, 0.1f, 0.0f, MAX_VALUE)) {
            game->player.camera.setPrespective(prespective, game->player.camera.getDepth());
        }

        Physbuzz::Camera::Depth depth = game->player.camera.getDepth();
        float depths[2] = {depth.near, depth.far};
        if (ImGui::DragFloat2("depth", depths, 1.0f, MIN_VALUE, MAX_VALUE)) {
            game->player.camera.setPrespective(
                prespective,
                {.near = depths[0], .far = depths[1]});
        }
    } break;

    // Orthographic
    case 1: {
        Physbuzz::Camera::Orthographic orthographic = game->player.camera.getOrthographic();

        if (ImGui::DragFloat("top", &orthographic.top, 1.0f, MIN_VALUE, MAX_VALUE)) {
            game->player.camera.setOrthographic(orthographic, game->player.camera.getDepth());
        }

        if (ImGui::DragFloat("bottom", &orthographic.bottom, 1.0f, MIN_VALUE, MAX_VALUE)) {
            game->player.camera.setOrthographic(orthographic, game->player.camera.getDepth());
        }

        if (ImGui::DragFloat("left", &orthographic.left, 1.0f, MIN_VALUE, MAX_VALUE)) {
            game->player.camera.setOrthographic(orthographic, game->player.camera.getDepth());
        }

        if (ImGui::DragFloat("right", &orthographic.right, 1.0f, MIN_VALUE, MAX_VALUE)) {
            game->player.camera.setOrthographic(orthographic, game->player.camera.getDepth());
        }

        Physbuzz::Camera::Depth depth = game->player.camera.getDepth();
        float depths[2] = {depth.near, depth.far};
        if (ImGui::DragFloat2("depth", depths, 1.0f, MIN_VALUE, MAX_VALUE)) {
            game->player.camera.setOrthographic(
                orthographic,
                {.near = depths[0], .far = depths[1]});
        }

    } break;

    // Orthographic2D
    case 2: {
        glm::ivec2 resolution = game->window.getResolution();
        ImGui::DragInt2("Resolution", glm::value_ptr(resolution), 1.0f, MIN_VALUE, MAX_VALUE); // 2D projection limited by window res
    } break;
    }

    ImGui::SeparatorText("View");

    if (ImGui::Button("Reset")) {
        game->player.camera.resetView();
    }

    glm::vec3 position = game->player.camera.getPosition();
    if (ImGui::DragFloat3("position", glm::value_ptr(position), 1.0f, MIN_VALUE, MAX_VALUE)) {
        game->player.camera.setPosition(position);
    }

    glm::vec3 up = game->player.camera.getUp();
    if (ImGui::DragFloat3("up", glm::value_ptr(up), 0.01f, MIN_VALUE, MAX_VALUE)) {
        game->player.camera.setUp(glm::normalize(up));
    }

    glm::vec3 facing = game->player.camera.getFacing();
    if (ImGui::DragFloat3("facing", glm::value_ptr(facing), 0.01f, MIN_VALUE, MAX_VALUE)) {
        game->player.camera.setFacing(glm::normalize(facing));
    }

    ImGui::SeparatorText("Player");

    ImGui::DragFloat("speed", &game->player.speed, 0.1f, 0.0f, MAX_VALUE);
    ImGui::DragFloat("senstivity", &game->player.sensitivity, 0.1f, 0.0f, MAX_VALUE);

    ImGui::End();
}
