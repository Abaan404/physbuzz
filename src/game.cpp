#include "game.hpp"

#include "objects/cube.hpp"
#include "objects/lightcube.hpp"
#include "objects/lightdirectional.hpp"
#include "objects/player.hpp"
#include "physbuzz/misc/clock.hpp"
#include "physbuzz/physics/dynamics.hpp"
#include "ui/handler.hpp"
#include <physbuzz/app/application.hpp>
#include <physbuzz/compat/imgui/imgui_impl_physbuzz.hpp>
#include <physbuzz/events/window.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/layout.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/renderers/forward.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/window/bindings.hpp>
#include <random>

struct TestVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static Physbuzz::VertexDescription Description;
};

Physbuzz::VertexDescription TestVertex::Description = {{
    .attributes = {
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(TestVertex::position) / sizeof(decltype(TestVertex::position)::value_type),
            .offset = offsetof(TestVertex, position),
        },
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(TestVertex::color) / sizeof(decltype(TestVertex::color)::value_type),
            .offset = offsetof(TestVertex, color),
        },
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32Sfloat,
            .size = sizeof(TestVertex::texCoord) / sizeof(decltype(TestVertex::texCoord)::value_type),
            .offset = offsetof(TestVertex, texCoord),
        },
    },
    .size = sizeof(TestVertex),
    .binding = 0,
}};

void Game::build() {
    Physbuzz::App::init();
    Physbuzz::Context::set(this);

    std::shared_ptr<Physbuzz::Window> window = Physbuzz::App::createWindow("main", {}, {1280, 720});

    // track cursor captures
    window->addCallback<Physbuzz::MousePositionEvent>([](const Physbuzz::MousePositionEvent &event) {
        static glm::vec2 lastPosition = event.window->getResolution() >> 1u;

        std::shared_ptr<Physbuzz::Renderer> renderer = Physbuzz::App::GScene.getSystem<Physbuzz::Renderer>();
        const auto [_, player, camera, flashlight] = Physbuzz::App::GScene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>().front();

        glm::vec2 offset = (static_cast<glm::vec2>(event.position) - lastPosition) * player.sensitivity;
        lastPosition = event.position;

        if (player.captureMouse || Physbuzz::App::GScene.getSystem<InterfaceManager>()->draw) {
            return;
        }

        event.window->setCursorCapture(true);

        const Physbuzz::CameraComponent::Info &info = camera.getInfo();
        glm::quat pitch = glm::angleAxis(glm::radians(offset.x), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::quat yaw = glm::angleAxis(glm::radians(offset.y), glm::cross(camera.getUp(), camera.getFacing()));

        camera.setOrientation(pitch * yaw * info.view.orientation);
        flashlight.direction = camera.getFacing();
    });

    // change prespective camera fov when scrolling
    window->addCallback<Physbuzz::MouseScrollEvent>([&](const Physbuzz::MouseScrollEvent &event) {
        std::shared_ptr<Physbuzz::Renderer> renderer = Physbuzz::App::GScene.getSystem<Physbuzz::Renderer>();
        const auto [_, player, camera] = Physbuzz::App::GScene.getComponents<PlayerComponent, Physbuzz::CameraComponent>().front();

        Physbuzz::CameraComponent::Info info = camera.getInfo();

        if (player.captureMouse || ImGui::GetIO().WantCaptureMouse || info.projection != Physbuzz::CameraComponent::Projection::Perspective) {
            return;
        }

        info.perspective.fovy = glm::clamp(info.perspective.fovy + glm::radians<float>(event.offset.y), glm::radians(30.0f), glm::radians(135.0f));

        camera.update(info);
    });

    Physbuzz::ObjectID playerId;

    {
        Player player = {
            .camera = {{
                .resolution = window->getResolution(),
                .projection = Physbuzz::CameraComponent::Projection::Perspective,
                .orthographic = {},
                .perspective = {
                    .fovy = glm::radians(45.0f),
                },
                .depth = {
                    .near = 1.0f,
                    .far = 10000.0f,
                },
                .view = {
                    .position = {0.0f, 0.0f, 2.0f},
                },
            }},
            .player = {},
        };

        playerId = ObjectBuilder::create(Physbuzz::App::GScene, player);
    }

    Physbuzz::App::GScene.createSystem<Physbuzz::Transfer>();
    Physbuzz::App::GScene.createSystem<Physbuzz::PipelineLayoutAllocator>(Physbuzz::PipelineLayoutAllocator::Info{});
    Physbuzz::App::GScene.createSystem<Physbuzz::Renderer>(Physbuzz::Renderer::Info{
        .window = window,
    });

    Physbuzz::App::GScene.getSystem<Physbuzz::Renderer>()->setRenderPasses({
        Physbuzz::App::GScene.createSystem<Physbuzz::ForwardRenderer>(Physbuzz::ForwardRenderer::Info{
            .camera = playerId,
        }),
        Physbuzz::App::GScene.createSystem<Physbuzz::ImGuiRenderer>(),
    });

    Physbuzz::App::GScene.createSystem<Physbuzz::Bindings>(window);
    Physbuzz::App::GScene.createSystem<Physbuzz::Clock>();
    Physbuzz::App::GScene.createSystem<Physbuzz::Dynamics>(1.0f);
    Physbuzz::App::GScene.createSystem<InterfaceManager>();

    std::random_device rd;
    std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(-250, 250);

    // cubes
    {
        for (int i = 0; i < 5; ++i) {
            Cube cube = {
                .cube = {
                    .width = 50.0f,
                    .height = 50.0f,
                    .length = 50.0f,
                },
                .transform = {
                    .position = {(i - 5 / 2) * 100, 0, 0},
                },
                .resources = {
                    .textures = {
                        {"crate/diffuse"},
                        {"crate/specular"},
                    },
                },
                .hasPhysics = false,
            };

            ObjectBuilder::create(Physbuzz::App::GScene, cube);
        }
    }

    // {
    //     for (int i = 0; i < 1; ++i) {
    //         LightCube lightCube = {
    //             .cube = {
    //                 .cube = {
    //                     .width = 10.0f,
    //                     .height = 10.0f,
    //                     .length = 10.0f,
    //                 },
    //                 .transform = {
    //                     .position = {distribution(rd), distribution(rd) + 250, distribution(rd)},
    //                     .orientation = glm::angleAxis(glm::radians(static_cast<float>(distribution(rd) % 360)), glm::normalize(glm::vec3(distribution(rd), distribution(rd), distribution(rd)))),
    //                 },
    //                 .identifier = {},
    //                 .resources = {
    //                     .textures = {
    //                         {"default/diffuse"},
    //                         {"default/specular"},
    //                     },
    //                 },
    //             },
    //             .pointLight = {.intensity = {1.0f, 1.0f, 0.0f}},
    //         };
    //
    //         ObjectBuilder::create(Physbuzz::App::GScene, lightCube);
    //     }
    // }

    {
        LightDirectional directional = {
            .directionalLight = {
                .direction = {1.0f, -1.0f, -1.0f},
                .intensity = {0.0f, 0.0f, 1.0f}},
        };

        ObjectBuilder::create(Physbuzz::App::GScene, directional);
    }
}

void Game::rebuild() {
}

void Game::loop() {
    m_IsRunning = true;

    const std::shared_ptr<Physbuzz::Window> &window = Physbuzz::App::getWindow("main");

    while (m_IsRunning && !window->shouldClose()) {
        Physbuzz::App::GScene.tickSystem<Physbuzz::Clock>();
        Physbuzz::App::GScene.tickSystem<Physbuzz::Dynamics>();
        Physbuzz::App::GScene.tickSystem<Physbuzz::Bindings>();
        Physbuzz::App::GScene.tickSystem<InterfaceManager, Physbuzz::Renderer>();
    }
}

void Game::destroy() {
    m_IsRunning = false;

    Physbuzz::App::quit();
}

int main() {
    Game game = Game();

    game.build();
    game.loop();
    game.destroy();

    return 0;
}
