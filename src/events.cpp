#include "events.hpp"

#include "game.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <physbuzz/context.hpp>
#include <physbuzz/events.hpp>

#include "objects/circle.hpp"
#include "objects/quad.hpp"
#include "objects/wall.hpp"

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
                for (auto &render : m_Game.scene.getComponents<Physbuzz::RenderComponent>()) {
                    render.destroy();
                }
            }

            m_Game.scene.clear();
            m_Game.collision.destroy();
            glm::ivec2 resolution = m_Game.window.getResolution();
            Wall wall = {
                .transform = {
                    .position = glm::vec3(resolution >> 1, 0.0f),
                },
                .wall = {
                    .width = static_cast<float>(resolution.x),
                    .height = static_cast<float>(resolution.y),
                    .thickness = 10.0f,
                },
                .isCollidable = true,
                .isRenderable = true,
            };

            m_Game.builder.create(wall);
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

            Quad info = {
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

            m_Game.builder.create(info);

        } break;

        case (GLFW_MOUSE_BUTTON_RIGHT): {
            glm::dvec2 cursor = m_Game.window.getCursorPos();

            Circle info = {
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

            m_Game.builder.create(info);

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
    glm::ivec2 resolution = {event.width, event.height};
    m_Game.renderer.getRenderer().resize(resolution);

    for (auto &object : m_Game.scene.getObjects()) {
        if (object.hasComponent<WallComponent>()) {
            Physbuzz::TransformableComponent &transform = object.getComponent<Physbuzz::TransformableComponent>();
            transform.position = glm::vec3(resolution >> 1, 0.0f);

            WallComponent &wall = object.getComponent<WallComponent>();
            wall.width = static_cast<float>(event.width),
            wall.height = static_cast<float>(event.height),

            object.getComponent<RebuildableComponent>().rebuild(m_Game.builder, object);
        }
    }

    std::vector<Physbuzz::RenderComponent> &renders = m_Game.scene.getComponents<Physbuzz::RenderComponent>();
    for (auto &render : renders) {
        render.mesh.isScaled = false;
    }
}

void Events::WindowClose(const Physbuzz::WindowCloseEvent &event) {
    m_Game.window.close();
}
