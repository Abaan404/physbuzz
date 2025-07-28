#include "game.hpp"

#include "collision.hpp"
#include "objects/builder.hpp"
#include "objects/circle.hpp"
#include "objects/cube.hpp"
#include "objects/lightcube.hpp"
#include "objects/lightdirectional.hpp"
#include "objects/model.hpp"
#include "objects/player.hpp"
#include "objects/quad.hpp"
#include "objects/skybox.hpp"
#include "resources/builder.hpp"
#include "resources/uniforms/time.hpp"
#include "resources/uniforms/window.hpp"
#include "ui/handler.hpp"
#include <imgui.h>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/framebuffer.hpp>
#include <physbuzz/render/gl/capabilities.hpp>
#include <physbuzz/render/renderers/deferred.hpp>
#include <physbuzz/render/renderers/forward.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/window/bindings.hpp>
#include <random>

void Game::build() {
    Physbuzz::Context::set(this);
    Physbuzz::Logger::build();

    window.build({1280, 720});

    ResourceBuilder resources;
    resources.build();

    // notify uniform of window
    window.addCallback<Physbuzz::WindowResizeEvent>([&](const Physbuzz::WindowResizeEvent &event) {
        Physbuzz::Resource<Physbuzz::UniformBuffer<UniformWindow>>("window")->update({
            .resolution = event.resolution,
        });

        scene.getSystem<Physbuzz::Renderer>()->resize(event.resolution);
        scene.getSystem<Physbuzz::Shadow>()->resize(event.resolution);
    });

    // track cursor captures
    window.addCallback<Physbuzz::MousePositionEvent>([&](const Physbuzz::MousePositionEvent &event) {
        static glm::vec2 lastPosition = event.window->getResolution() >> 1;

        std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
        const auto [player, camera, flashlight] = scene.getComponent<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>(renderer->getInfo().camera);

        glm::vec2 offset = (static_cast<glm::vec2>(event.position) - lastPosition) * player.sensitivity;
        lastPosition = event.position;

        if (player.captureMouse || scene.getSystem<InterfaceManager>()->draw) {
            return;
        }

        window.setCursorCapture(true);

        const Physbuzz::CameraComponent::Info &info = camera.getInfo();
        glm::quat pitch = glm::angleAxis(glm::radians(offset.x), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::quat yaw = glm::angleAxis(glm::radians(offset.y), glm::cross(camera.getUp(), camera.getFacing()));

        camera.setOrientation(pitch * yaw * info.view.orientation);
        flashlight.direction = camera.getFacing();
    });

    // change prespective camera fov when scrolling
    window.addCallback<Physbuzz::MouseScrollEvent>([&](const Physbuzz::MouseScrollEvent &event) {
        std::shared_ptr<Physbuzz::Renderer> renderer = scene.getSystem<Physbuzz::Renderer>();
        const auto [player, camera] = scene.getComponent<PlayerComponent, Physbuzz::CameraComponent>(renderer->getInfo().camera);

        Physbuzz::CameraComponent::Info info = camera.getInfo();

        if (player.captureMouse || ImGui::GetIO().WantCaptureMouse || info.projection != Physbuzz::CameraComponent::Projection::Perspective) {
            return;
        }

        info.perspective.fovy = glm::clamp(info.perspective.fovy + glm::radians<float>(event.offset.y), glm::radians(30.0f), glm::radians(135.0f));

        camera.update(info);
    });

    // enable backface culling and depth testing
    Physbuzz::GL::setCapability(Physbuzz::GL::Capabilities::CullFace, true);
    Physbuzz::GL::setCapability(Physbuzz::GL::Capabilities::DepthTest, true);

    // Create a default scene
    rebuild();
}

void Game::rebuild() {
    Physbuzz::CameraComponent restoreCamera = {{}};
    for (const auto &[_1, _2, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
        restoreCamera = camera;
    }

    scene.clear();

    std::random_device rd;
    std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(-100, 100);

    Physbuzz::ObjectID playerObject;

    // player
    {
        Player player = {
            .camera = {{
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
                    .position = {0.0f, 50.0f, 0.0f},
                },
                .resolution = window.getResolution(),
            }},
            .player = {},
        };

        if (restoreCamera.getInfo().projection != Physbuzz::CameraComponent::Projection::Unknown) {
            player.camera = {restoreCamera};
        }

        playerObject = ObjectBuilder::create(scene, player);
    }

    // skybox
    {
        Skybox skybox;
        ObjectBuilder::create(scene, skybox);
    }

    // backpack
    {
        Model backpack = {
            .model = {
                .resource = {"backpack"},
            },
            .transform = {
                .position = glm::vec3(0.0f, 50.0f, 0.0f),
                .scale = glm::vec3(20.0f, 20.0f, 20.0f),
                .orientation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            },
            .resources = {},
        };

        ObjectBuilder::create(scene, backpack);
    }

    // platform
    {
        Quad quad = {
            .body = {},
            .quad = {
                .width = 5000.0f,
                .height = 5000.0f,
            },
            .transform = {
                .orientation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            },
            .resources = {
                .textures = {
                    {"floor"},
                    {"default/specular"},
                },
            },
        };

        ObjectBuilder::create(scene, quad);
    }

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
                    .position = {distribution(rd), distribution(rd) + 250, distribution(rd)},
                    .orientation = glm::angleAxis(glm::radians(static_cast<float>(distribution(rd) % 360)), glm::normalize(glm::vec3(distribution(rd), distribution(rd), distribution(rd)))),
                },
                .resources = {
                    .textures = {
                        {"crate/diffuse"},
                        {"crate/specular"},
                    },
                },
                .hasPhysics = false,
            };

            ObjectBuilder::create(scene, cube);
        }
    }

    // point lights
    {
        for (int i = 0; i < 1; ++i) {
            LightCube lightCube = {
                .cube = {
                    .cube = {
                        .width = 10.0f,
                        .height = 10.0f,
                        .length = 10.0f,
                    },
                    .transform = {
                        .position = {distribution(rd), distribution(rd) + 250, distribution(rd)},
                        .orientation = glm::angleAxis(glm::radians(static_cast<float>(distribution(rd) % 360)), glm::normalize(glm::vec3(distribution(rd), distribution(rd), distribution(rd)))),
                    },
                    .identifier = {},
                    .resources = {
                        .textures = {
                            {"default/diffuse"},
                            {"default/specular"},
                        },
                    },
                },
                .pointLight = {
                    .ambient = {0.2f, 0.2f, 0.2f},
                    .diffuse = {0.8f, 0.8f, 0.8f},
                    .specular = {1.0f, 1.0f, 1.0f},

                    .constant = 1.0f,
                    .linear = 0.01f,
                    .quadratic = 0.0001f,
                },
            };

            ObjectBuilder::create(scene, lightCube);
        }
    }

    // directional light
    {
        LightDirectional directional = {
            .directionalLight = {
                .direction = {1.0f, -1.0f, -1.0f},

                .ambient = {0.2f, 0.2f, 0.2f},
                .diffuse = {0.5f, 0.5f, 0.5f},
                .specular = {0.5f, 0.5f, 0.5f},
            },
        };

        ObjectBuilder::create(scene, directional);
    }

    // a circle because why not?
    {
        Circle point = {
            .body = {},
            .circle = {
                .radius = 10.0f,
            },
            .transform = {
                .position = {100.0f, 100.0f, 100.0f},
            },
            .resources = {
                .textures = {
                    {"default/diffuse"},
                    {"default/specular"},
                },
                .pipeline = {"circle"},
            },
        };

        ObjectBuilder::create(scene, point);
    }

    // ticking systems
    {
        scene.createSystem<Collision>(&scene, 0.9);
        scene.createSystem<Physbuzz::Dynamics>(0.0005);
        scene.createSystem<Physbuzz::Bindings>(&window);
        scene.createSystem<Physbuzz::Clock>();
        scene.createSystem<Physbuzz::Shadow>(Physbuzz::Shadow::Info{
            .resolution = window.getResolution(),
            .orthoSize = 1000.0f,
            .depth = 10000.0f,
        });
        scene.createSystem<Physbuzz::Renderer>(Physbuzz::Renderer::Info{
            .type = Physbuzz::Renderer::Type::Deferred,
            .camera = playerObject,
            .resolution = window.getResolution(),
            .postProcessing = {
                {"gamma"},
            },
        });
        scene.createSystem<InterfaceManager>(InterfaceManager::Info{
            .window = &window,
        });
    }
}

void Game::loop() {
    m_IsRunning = true;

    while (m_IsRunning && !window.shouldClose()) {
        auto clock = scene.getSystem<Physbuzz::Clock>();
        Physbuzz::Resource<Physbuzz::UniformBuffer<UniformTime>>("time")->update({
            .time = clock->getTime(),
            .timedelta = clock->getDelta(),
        });

        // scene.tickSystem<Physbuzz::Dynamics, Collision>();
        scene.tickSystem<Physbuzz::Bindings>();
        scene.tickSystem<Physbuzz::Clock>();
        scene.tickSystem<Physbuzz::Shadow, Physbuzz::Renderer>();
        scene.tickSystem<InterfaceManager>();
        window.flip();
    }
}

void Game::destroy() {
    ResourceBuilder resources;
    resources.destroy();

    Physbuzz::ResourceRegistry<Physbuzz::Model>::clear(); // clean up generated models

    m_IsRunning = false;

    scene.clear();
    window.destroy();
}

int main() {
    Game game = Game();

    game.build();
    game.loop();
    game.destroy();

    return 0;
}
