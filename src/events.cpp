#include "events.hpp"

#include "game.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <physbuzz/context.hpp>
#include <physbuzz/events.hpp>

#include "objects/circle.hpp"
#include "objects/quad.hpp"

Events::Events(Game &game)
    : m_Game(game) {}

Events::~Events() {}

void Events::build() {
    m_Game.window.setCallback<Physbuzz::KeyEvent>([&](const Physbuzz::KeyEvent &event) { keyEvent(event); });
    m_Game.window.setCallback<Physbuzz::MouseButtonEvent>([&](const Physbuzz::MouseButtonEvent &event) { mouseButton(event); });
    m_Game.window.setCallback<Physbuzz::WindowResizeEvent>([&](const Physbuzz::WindowResizeEvent &event) { WindowResize(event); });
    m_Game.window.setCallback<Physbuzz::WindowCloseEvent>([&](const Physbuzz::WindowCloseEvent &event) { WindowClose(event); });
}

void Events::destroy() {}

void Events::keyEvent(const Physbuzz::KeyEvent &event) {
    if (ImGui::GetIO().WantCaptureKeyboard && event.key != GLFW_KEY_F3) {
        return;
    }

    switch (event.action) {
    case (GLFW_PRESS): {
        switch (event.key) {
        case (GLFW_KEY_F3): {
            m_Game.interface.draw ^= true;
        } break;

        case (GLFW_KEY_C): {
            if (m_Game.scene.existsComponents<Physbuzz::RenderComponent>()) {
                for (auto &mesh : m_Game.scene.getComponents<Physbuzz::RenderComponent>()) {
                    mesh.destroy();
                }
            }

            m_Game.wall.destroy();
            m_Game.scene.clear();
            m_Game.collision.destroy();

            const WallInfo &info = m_Game.wall.getInfo();
            m_Game.wall.build(info);
            m_Game.collision.build();
        } break;
        }

    } break;
    }
}

void Events::mouseButton(const Physbuzz::MouseButtonEvent &event) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    switch (event.action) {
    case (GLFW_PRESS): {
        switch (event.button) {
        case (GLFW_MOUSE_BUTTON_LEFT): {
            glm::dvec2 cursor = m_Game.window.getCursorPos();

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

            ObjectBuilder<QuadInfo>::build(m_Game.scene, info);

        } break;

        case (GLFW_MOUSE_BUTTON_RIGHT): {
            glm::dvec2 cursor = m_Game.window.getCursorPos();

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

            ObjectBuilder<CircleInfo>::build(m_Game.scene, info);

        } break;

        default:
            break;
        }

    } break;

    default:
        break;
    }
}

void Events::WindowResize(const Physbuzz::WindowResizeEvent &event) {
    Game *game = Physbuzz::Context::get<Game>(event.window);

    glm::ivec2 resolution = {event.width, event.height};
    m_Game.renderer.resize(resolution);

    if (m_Game.wall.isErect()) {
        WallInfo info = m_Game.wall.getInfo();
        info.transform = {
            .position = glm::vec3(resolution >> 1, 0.0f),
        };

        info.wall = {
            .width = static_cast<float>(event.width),
            .height = static_cast<float>(event.height),
            .thickness = info.wall.thickness,
        };

        m_Game.wall.destroy();
        m_Game.wall.build(info);
    }

    std::vector<Physbuzz::MeshComponent> &meshes = m_Game.scene.getComponents<Physbuzz::MeshComponent>();
    for (auto &mesh : meshes) {
        mesh.renormalize();
    }
}

void Events::WindowClose(const Physbuzz::WindowCloseEvent &event) {
    Game *game = Physbuzz::Context::get<Game>(event.window);
    m_Game.window.close();
}
