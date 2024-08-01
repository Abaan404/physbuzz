#include "player.hpp"

#include "bindings.hpp"
#include "game.hpp"
#include "objects/cube.hpp"
#include <imgui.h>

Player::Player(Game *game)
    : m_Game(game) {}

Player::~Player() {}

void Player::resize(const glm::ivec2 &resolution) {
    camera.setPrespective(
        {.fovy = glm::radians(45.0f), .aspect = static_cast<float>(resolution.x) / static_cast<float>(resolution.y)},
        {.near = 1.0f, .far = 1000.0f});
}

void Player::build() {
    camera.build();
    resize(m_Game->window.getResolution());

    m_Game->bindings.mouseButtonCallbacks[Physbuzz::Mouse::Left] = {
        .callback = [&]() {
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
        .type = BindingType::OneShot,
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::F3] = {
        .callback = [&]() {
            m_Game->interface.draw ^= true;
        },
        .type = BindingType::OneShot,
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::C] = {
        .callback = [&]() {
            if (m_Game->scene.existsComponents<Physbuzz::RenderComponent>()) {
                for (auto &render : m_Game->scene.getComponents<Physbuzz::RenderComponent>()) {
                    render.destroy();
                }
            }

            m_Game->scene.clear();
            m_Game->collision.destroy();

            m_Game->collision.build();
        },
        .type = BindingType::OneShot,
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::W] = {
        .callback = [&]() {
            camera.setPosition(camera.getPosition() + m_Speed * camera.getFacing());
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::A] = {
        .callback = [&]() {
            camera.setPosition(camera.getPosition() - m_Speed * glm::normalize(glm::cross(camera.getFacing(), camera.getUp())));
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::S] = {
        .callback = [&]() {
            camera.setPosition(camera.getPosition() - m_Speed * camera.getFacing());
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::D] = {
        .callback = [&]() {
            camera.setPosition(camera.getPosition() + m_Speed * glm::normalize(glm::cross(camera.getFacing(), camera.getUp())));
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::Space] = {
        .callback = [&]() {
            camera.setPosition(camera.getPosition() + m_Speed * camera.getUp());
        },
    };

    m_Game->bindings.keyboardCallbacks[Physbuzz::Key::LeftShift] = {
        .callback = [&]() {
            camera.setPosition(camera.getPosition() - m_Speed * camera.getUp());
        },
    };
}

void Player::destroy() {
    camera.destroy();
}
