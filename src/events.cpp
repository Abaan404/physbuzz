#include "events.hpp"

#include "game.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <physbuzz/context.hpp>
#include <physbuzz/events.hpp>

#include "objects/circle.hpp"
#include "objects/quad.hpp"

void Events::keyEvent(Physbuzz::KeyEvent event) {
    Game *game = Physbuzz::Context::get<Game>(event.window);

    ImGui_ImplGlfw_KeyCallback(event.window, event.key, event.scancode, event.action, event.mods);

    if (ImGui::GetIO().WantCaptureKeyboard && event.key != GLFW_KEY_F3) {
        return;
    }

    switch (event.action) {
    case (GLFW_PRESS): {
        switch (event.key) {
        case (GLFW_KEY_F3): {
            game->interface.draw ^= true;
        } break;

        case (GLFW_KEY_C): {
            if (game->scene.existsComponents<Physbuzz::RenderComponent>()) {
                for (auto &mesh : game->scene.getComponents<Physbuzz::RenderComponent>()) {
                    mesh.destroy();
                }
            }

            game->wall.destroy();
            game->scene.clear();
            game->collision.destroy();

            const WallInfo &info = game->wall.getInfo();
            game->wall.build(info);
            game->collision.build();
        } break;
        }

    } break;
    }
}

void Events::mouseButton(Physbuzz::MouseButtonEvent event) {
    Game *game = Physbuzz::Context::get<Game>(event.window);

    ImGui_ImplGlfw_MouseButtonCallback(event.window, event.button, event.action, event.mods);

    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    switch (event.action) {
    case (GLFW_PRESS): {
        switch (event.button) {
        case (GLFW_MOUSE_BUTTON_LEFT): {
            glm::dvec2 cursor = game->window.getCursorPos();

            QuadInfo info = {
                .body = {
                    .gravity = {
                        .acceleration = {0.0f, 1000.0f, 0.0f},
                    },
                },
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

            ObjectBuilder<QuadInfo>::build(game->scene, info);

        } break;

        case (GLFW_MOUSE_BUTTON_RIGHT): {
            glm::dvec2 cursor = game->window.getCursorPos();

            CircleInfo info = {
                .body = {
                    // .velocity = {0.0f, 0.01f, 0.0f},
                    .gravity = {
                        .acceleration = {0.0f, 1000.0f, 0.0f},
                    },
                },
                .transform = {
                    .position = {cursor.x, cursor.y, 0.0f},
                },
                .circle = {
                    .radius = 50.0f,
                },
                .isCollidable = true,
                .isRenderable = true,
            };

            ObjectBuilder<CircleInfo>::build(game->scene, info);

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
    Game *game = Physbuzz::Context::get<Game>(event.window);

    glm::ivec2 resolution = {event.width, event.height};
    game->renderer.resize(resolution);

    if (game->wall.isErect()) {
        WallInfo info = game->wall.getInfo();
        info.transform = {
            .position = glm::vec3(resolution >> 1, 0.0f),
        };

        info.wall = {
            .width = static_cast<float>(event.width),
            .height = static_cast<float>(event.height),
            .thickness = info.wall.thickness,
        };

        game->wall.destroy();
        game->wall.build(info);
    }

    std::vector<Physbuzz::MeshComponent> &meshes = game->scene.getComponents<Physbuzz::MeshComponent>();
    for (auto &mesh : meshes) {
        mesh.renormalize();
    }
}

void Events::WindowClose(Physbuzz::WindowCloseEvent event) {
    Game *game = Physbuzz::Context::get<Game>(event.window);
    game->window.close();
}
