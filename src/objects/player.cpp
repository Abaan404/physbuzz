#include "player.hpp"

#include "../game.hpp"
#include "../ui/handler.hpp"
#include "circle.hpp"
#include "cuboid.hpp"
#include <imgui.h>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/window/inputs.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Player &info) {
    if (scene.getComponents<PlayerComponent>().size() >= 1) {
        Physbuzz::Logger::WARNING("[ObjectBuilder<Player>] Trying to build more than one player controller. Are you sure? Pretty sure? Threw a trashbag into space?");
    }

    info.flashlight.direction = info.camera.getFacing();

    // setup inputs
    Physbuzz::InputEventComponent inputs = {
        .keyboardCallbacks = {
            {
                .key = Physbuzz::Key::F3,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &event) {
                    std::shared_ptr<InterfaceManager> interface = scene.getSystem<InterfaceManager>();

                    interface->draw ^= true;
                    if (interface->draw) {
                        event.window->setCursorCapture(false);
                    }
                },
            },
            {
                .key = Physbuzz::Key::Escape,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &event) {
                    std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();

                    const auto [_, player] = scene.getComponents<PlayerComponent>().front();
                    player.captureMouse ^= true;

                    event.window->setCursorCapture(false);
                },
            },
            {
                .key = Physbuzz::Key::C,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](Physbuzz::Scene &, const Physbuzz::KeyEvent &) {
                    Game *game = Physbuzz::Context::get<Game>();
                    game->rebuild();
                },
            },
            {
                .key = Physbuzz::Key::W,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position + camera.getFacing() * player.speed);
                    flashlight.position = camera.getInfo().view.position;
                },
            },
            {
                .key = Physbuzz::Key::A,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position - camera.getRight() * player.speed);
                    flashlight.position = camera.getInfo().view.position;
                },
            },
            {
                .key = Physbuzz::Key::S,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position - camera.getFacing() * player.speed);
                    flashlight.position = camera.getInfo().view.position;
                },
            },
            {
                .key = Physbuzz::Key::D,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position + camera.getRight() * player.speed);
                    flashlight.position = camera.getInfo().view.position;
                },
            },
            {
                .key = Physbuzz::Key::LeftShift,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position - camera.getUp() * player.speed);
                    flashlight.position = camera.getInfo().view.position;
                },
            },
            {
                .key = Physbuzz::Key::Space,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position + camera.getUp() * player.speed);
                    flashlight.position = camera.getInfo().view.position;
                },
            },
        },
        .mouseCallbacks = {
            {
                .button = Physbuzz::Button::Left,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::MouseButtonEvent &) {
                    if (ImGui::GetIO().WantCaptureMouse) {
                        return;
                    }

                    Cuboid info = {
                        .cuboid = {
                            .width = 100.0f,
                            .breadth = 100.0f,
                            .height = 100.0f,
                        },
                        .transform = {
                            .position = {0.0f, 0.0f, 0.0f},
                            .orientation = glm::angleAxis(glm::pi<float>() / 4.0f, glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f))),
                        },
                        .resources = {},
                        .hasPhysics = false,
                    };

                    ObjectBuilder::create(scene, info);
                },
            },
            {
                .button = Physbuzz::Button::Right,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::MouseButtonEvent &event) {
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

                    ObjectBuilder::create(scene, info);
                },
            },
        },
    };

    scene.setComponent(object, inputs, info.player, info.camera, info.identifier, info.flashlight);

    return object;
}
