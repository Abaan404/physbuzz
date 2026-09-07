#include "player.hpp"

#include "../game.hpp"
#include "../ui/handler.hpp"
#include <imgui.h>
#include <physbuzz/graphics/renderer.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/window/inputs.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Player &info) {
    if (scene.getComponents<PlayerComponent>().size() >= 1) {
        Physbuzz::Logger::WARNING("[ObjectBuilder<Player>] Trying to build more than one player controller. Are you sure? Pretty sure? Threw a trashbag into space?");
    }

    info.flashlight.setDirection(info.camera.getFacing());

    // setup inputs
    Physbuzz::InputEventComponent inputs = {
        .keyboardCallbacks = {
            {
                .key = Physbuzz::Key::F3,
                .type = Physbuzz::CallbackType::OneShot,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &event) {
                    InterfaceManager *interface = scene.getSystem<InterfaceManager>();

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
                    Physbuzz::Renderer *renderer = scene.getSystem<Physbuzz::Renderer>();

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
                    Physbuzz::Renderer *renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position + camera.getFacing() * player.speed);
                    flashlight.setPosition(camera.getInfo().view.position);
                },
            },
            {
                .key = Physbuzz::Key::A,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    Physbuzz::Renderer *renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position - camera.getRight() * player.speed);
                    flashlight.setPosition(camera.getInfo().view.position);
                },
            },
            {
                .key = Physbuzz::Key::S,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    Physbuzz::Renderer *renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position - camera.getFacing() * player.speed);
                    flashlight.setPosition(camera.getInfo().view.position);
                },
            },
            {
                .key = Physbuzz::Key::D,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    Physbuzz::Renderer *renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position + camera.getRight() * player.speed);
                    flashlight.setPosition(camera.getInfo().view.position);
                },
            },
            {
                .key = Physbuzz::Key::LeftShift,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    Physbuzz::Renderer *renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position - camera.getUp() * player.speed);
                    flashlight.setPosition(camera.getInfo().view.position);
                },
            },
            {
                .key = Physbuzz::Key::Space,
                .type = Physbuzz::CallbackType::Continous,
                .callback = [](Physbuzz::Scene &scene, const Physbuzz::KeyEvent &) {
                    Physbuzz::Renderer *renderer = scene.getSystem<Physbuzz::Renderer>();
                    const auto [_, player, camera, flashlight] = scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

                    camera.setPosition(camera.getInfo().view.position + camera.getUp() * player.speed);
                    flashlight.setPosition(camera.getInfo().view.position);
                },
            },
        },
    };

    scene.setComponent(object, inputs, info.player, info.camera, info.identifier, info.flashlight);

    return object;
}
