#include "player.hpp"

#include "../game.hpp"
#include "circle.hpp"
#include "cube.hpp"
#include <imgui.h>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/gl/capabilities.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Player &info) {
    if (scene->getComponents<PlayerComponent>().size() >= 1) {
        Physbuzz::Logger::WARNING("[ObjectBuilder<Player>] Trying to build more than one player controller. Are you sure? Pretty sure? Threw a trashbag into space?");
    }

    // setup inputs
    Physbuzz::BindingComponent inputs = {
        .keyboardCallbacks = {
            {
                .key = Physbuzz::Key::F3,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](const Physbuzz::KeyEvent &event) {
                    Game *game = Physbuzz::Context::get<Game>();

                    game->interface.draw ^= true;
                    if (game->interface.draw) {
                        event.window->setCursorCapture(false);
                    }
                },
            },
            {
                .key = Physbuzz::Key::Escape,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](const Physbuzz::KeyEvent &event) {
                    Game *game = Physbuzz::Context::get<Game>();

                    for (const auto &[player, camera] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
                        player.captureMouse ^= true;
                    }

                    event.window->setCursorCapture(false);
                },
            },
            {
                .key = Physbuzz::Key::C,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();
                    game->rebuild();
                },
            },
            {
                .key = Physbuzz::Key::W,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();

                    for (const auto &[player, camera] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
                        camera.setPosition(camera.getInfo().view.position + camera.getFacing() * player.speed);
                    }
                },
            },
            {
                .key = Physbuzz::Key::A,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();

                    for (const auto &[player, camera] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
                        camera.setPosition(camera.getInfo().view.position - camera.getRight() * player.speed);
                    }
                },
            },
            {
                .key = Physbuzz::Key::S,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();

                    for (const auto &[player, camera] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
                        camera.setPosition(camera.getInfo().view.position - camera.getFacing() * player.speed);
                    }
                },
            },
            {
                .key = Physbuzz::Key::D,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();

                    for (const auto &[player, camera] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
                        camera.setPosition(camera.getInfo().view.position + camera.getRight() * player.speed);
                    }
                },
            },
            {
                .key = Physbuzz::Key::LeftShift,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();

                    for (const auto &[player, camera] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
                        camera.setPosition(camera.getInfo().view.position - camera.getUp() * player.speed);
                    }
                },
            },
            {
                .key = Physbuzz::Key::Space,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();

                    for (const auto &[player, camera] : game->scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
                        camera.setPosition(camera.getInfo().view.position + camera.getUp() * player.speed);
                    }
                },
            },
        },
        .mouseCallbacks = {
            {
                .button = Physbuzz::Button::Left,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](const Physbuzz::MouseButtonEvent &) {
                    if (ImGui::GetIO().WantCaptureMouse) {
                        return;
                    }

                    Cube info = {
                        .cube = {
                            .width = 100.0f,
                            .height = 100.0f,
                            .length = 100.0f,
                        },
                        .transform = {
                            .position = {0.0f, 0.0f, 0.0f},
                            .orientation = glm::angleAxis(glm::pi<float>() / 4.0f, glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f))),
                        },
                        .resources = {},
                        .hasPhysics = false,
                    };

                    Game *game = Physbuzz::Context::get<Game>();
                    game->builder.create(info);
                },
            },
            {
                .button = Physbuzz::Button::Right,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](const Physbuzz::MouseButtonEvent &event) {
                    if (ImGui::GetIO().WantCaptureMouse) {
                        return;
                    }

                    const glm::ivec2 &cursor = event.window->getCursorPos();
                    Circle info = {
                        .body = {
                            .angular = {},
                            .gravity = {
                                .acceleration = {0.0f, 1000.0f, 0.0f},
                            },
                            .drag = {},
                        },
                        .circle = {
                            .radius = 100.0f,
                        },
                        .transform = {
                            .position = {cursor.x, cursor.y, 0.0f},
                        },
                        .resources = {},
                        .hasPhysics = true,
                    };

                    Game *game = Physbuzz::Context::get<Game>();
                    game->builder.create(info);
                },
            },
        },
    };

    scene->setComponent(object, inputs, info.player, info.camera, info.identifier, info.flashlight);

    return object;
}
