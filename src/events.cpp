#include "events.hpp"

#include "game.hpp"
#include "objects/circle.hpp"
#include "objects/quad.hpp"
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
            Game::wall.rebuild();
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

            QuadInfo info = {
                .transform = {
                    .position = {cursor.x, cursor.y, 0.0f},
                },
                .quad = {
                    .width = 10.0f,
                    .height = 10.0f,
                },
                .isCollidable = true,
                .isRenderable = true,
            };

            ObjectBuilder<QuadInfo>::build(Game::scene, info);

        } break;

        case (GLFW_MOUSE_BUTTON_RIGHT): {
            glm::dvec2 cursor = Game::window.getCursorPos();

            CircleInfo info = {
                .transform = {
                    .position = {cursor.x, cursor.y, 0.0f},
                },
                .circle = {
                    .radius = 20.0f,
                },
                .isCollidable = true,
                .isRenderable = true,
            };

            ObjectBuilder<CircleInfo>::build(Game::scene, info);

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
    glm::ivec2 resolution = {event.width, event.height};
    Game::renderer.resize(resolution);

    if (Game::wall.isErect()) {
        Physbuzz::Object &wall = Game::scene.getObject(Game::wall.getId());
        WallInfo info = {
            .transform = {
                .position = glm::vec3(resolution >> 1, 0.0f),
                .scale = wall.getComponent<TransformableComponent>().scale},
            .wall = {
                .width = static_cast<float>(event.width),
                .height = static_cast<float>(event.height),
                .thickness = wall.getComponent<WallComponent>().thickness,
            },
            .isCollidable = true,
            .isRenderable = false,
        };

        Game::wall.build(info);
    }

    std::vector<Physbuzz::MeshComponent> &meshes = Game::scene.getComponents<Physbuzz::MeshComponent>();
    for (auto &mesh : meshes) {
        mesh.redraw();
    }
}

void Events::WindowClose(Physbuzz::WindowCloseEvent event) {
    Game::isRunning = false;
}
