#include "player.hpp"

#include "game.hpp"
#include "objects/circle.hpp"
#include "objects/cube.hpp"
#include <imgui.h>

Player::Player(Game *game)
    : m_Game(game) {}

Player::~Player() {}

void Player::resize(const glm::ivec2 &resolution) {
    camera.resize(resolution, camera.getDepth());
}

void Player::build() {
    camera.build();
    camera.setPrespective(
        {.fovy = glm::radians(45.0f), .aspect = static_cast<float>(m_Game->window.getResolution().x) / static_cast<float>(m_Game->window.getResolution().y)},
        {.near = 1.0f, .far = 1000.0f});

    m_Game->bindings.mouseButtonCallbacks[Physbuzz::Mouse::Left] = {
        .callback = [&](const Physbuzz::MouseButtonEvent &event) {
            if (ImGui::GetIO().WantCaptureMouse) {
                return;
            }

            Cube info = {
                .transform = {
                    .position = {0.0f, 0.0f, 0.0f},
                    .orientation = glm::angleAxis(glm::pi<float>() / 4.0f, glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f))),
                },
                .cube = {
                    .width = 100.0f,
                    .height = 100.0f,
                    .length = 100.0f,
                },
                .isCollidable = true,
                .isRenderable = true,
            };
            m_Game->builder.create(info);
        },
        .type = Physbuzz::CallbackType::OneShot,
    };

    m_Game->bindings.mouseButtonCallbacks[Physbuzz::Mouse::Right] = {
        .callback = [&](const Physbuzz::MouseButtonEvent &event) {
            if (ImGui::GetIO().WantCaptureMouse) {
                return;
            }

            glm::ivec2 cursor = m_Game->window.getCursorPos();
            Circle info = {
                .body = {
                    .gravity = {
                        .acceleration = {0.0f, 1000.0f, 0.0f},
                    },
                },
                .transform = {
                    .position = {cursor.x, cursor.y, 0.0f},
                },
                .circle = {
                    .radius = 100.0f,
                },
                .resources = {
                    .texture2D = "wall",
                },
                .isCollidable = true,
                .isRenderable = true,
            };
            m_Game->builder.create(info);
        },
        .type = Physbuzz::CallbackType::OneShot,
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::F3] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            m_Game->interface.draw ^= true;
            if (m_Game->interface.draw) {
                m_Game->window.setCursorCapture(false);
            }
        },
        .type = Physbuzz::CallbackType::OneShot,
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::C] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            if (m_Game->scene.existsComponents<Physbuzz::Mesh>()) {
                for (auto &mesh : m_Game->scene.getComponents<Physbuzz::Mesh>()) {
                    mesh.destroy();
                }
            }

            m_Game->scene.clear();
            m_Game->collision.destroy();

            m_Game->collision.build();
        },
        .type = Physbuzz::CallbackType::OneShot,
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::W] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            camera.translate(camera.getFacing() * speed);
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::A] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            camera.translate(-camera.getRight() * speed);
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::S] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            camera.translate(-camera.getFacing() * speed);
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::D] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            camera.translate(camera.getRight() * speed);
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::Space] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            camera.translate(camera.getUp() * speed);
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::LeftShift] = {
        .callback = [&](const Physbuzz::KeyEvent &event) {
            camera.translate(-camera.getUp() * speed);
        },
    };

    m_Game->window.addCallback<Physbuzz::MousePositionEvent>([&](const Physbuzz::MousePositionEvent &event) {
        if (ImGui::GetIO().WantCaptureMouse || m_Game->interface.draw || camera.getProjectionType() == Physbuzz::Camera::ProjectionType::Orthographic2D) {
            return;
        }

        m_Game->window.setCursorCapture(true);

        static glm::vec2 lastPosition = m_Game->window.getResolution() >> 1;
        glm::vec2 offset = (static_cast<glm::vec2>(event.position) - lastPosition) * sensitivity;
        lastPosition = event.position;

        glm::quat pitch = glm::angleAxis(glm::radians(offset.x), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::quat yaw = glm::angleAxis(glm::radians(offset.y), glm::cross(camera.getUp(), camera.getFacing()));

        camera.rotate(pitch * yaw);
    });

    m_Game->window.addCallback<Physbuzz::MouseScrollEvent>([&](const Physbuzz::MouseScrollEvent &event) {
        if (ImGui::GetIO().WantCaptureMouse || camera.getProjectionType() != Physbuzz::Camera::ProjectionType::Prespective) {
            return;
        }

        Physbuzz::Camera::Prespective prespective = camera.getPrespective();
        prespective.fovy = glm::clamp(prespective.fovy + glm::radians<float>(event.offset.y), glm::radians(30.0f), glm::radians(135.0f));

        camera.setPrespective(prespective, camera.getDepth());
    });
}

void Player::destroy() {
    camera.destroy();
}
