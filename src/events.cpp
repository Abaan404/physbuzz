#include "events.hpp"

#include "game.hpp"
#include "objects/objects.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <physbuzz/events.hpp>

void Events::keyEvent(Physbuzz::KeyEvent event) {
    ImGui_ImplGlfw_KeyCallback(Game::window.getWindow(), event.key, event.scancode, event.action, event.mods);

    if (ImGui::GetIO().WantCaptureKeyboard && event.key != GLFW_KEY_F3) {
        return;
    }

    switch (event.action) {
    case (GLFW_PRESS): {
        switch (event.key) {
        case (GLFW_KEY_F3): {
            Game::interface.draw ^= true;
        } break;

        case (GLFW_KEY_C): {
            if (Game::scene.existsComponents<Physbuzz::MeshComponent>()) {
                for (auto &mesh : Game::scene.getComponents<Physbuzz::MeshComponent>()) {
                    mesh.destroy();
                }
            }

            Game::scene.clear();
        } break;
        }

    } break;
    }
}

void Events::mouseButton(Physbuzz::MouseButtonEvent event) {
    ImGui_ImplGlfw_MouseButtonCallback(Game::window.getWindow(), event.button, event.action, event.mods);

    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    switch (event.action) {
    case (GLFW_PRESS): {
        switch (event.button) {
        case (GLFW_MOUSE_BUTTON_LEFT): {
            glm::dvec2 cursor = Game::window.getCursorPos();
            Physbuzz::ObjectID id = Game::scene.createObject();

            buildBox(Game::scene.getObject(id), glm::vec3(cursor.x, cursor.y, 0.0f), 10, 10);

        } break;

        case (GLFW_MOUSE_BUTTON_RIGHT): {
            glm::dvec2 cursor = Game::window.getCursorPos();
            Physbuzz::ObjectID id = Game::scene.createObject();

            buildCircle(Game::scene.getObject(id), glm::vec3(cursor.x, cursor.y, 0.0f), 20);

        } break;

        default:
            break;
        }

    } break;

    default:
        break;
    }
}

void Events::WindowResize(Physbuzz::WindowResizeEvent event) {
    glm::ivec2 resolution{event.width, event.height};
    Game::renderer.resize(resolution);

    std::vector<Physbuzz::MeshComponent> &meshes = Game::scene.getComponents<Physbuzz::MeshComponent>();
    for (auto &mesh : meshes) {
        mesh.redraw();
    }
}

void Events::WindowClose(Physbuzz::WindowCloseEvent event) {
    Game::isRunning = false;
}
