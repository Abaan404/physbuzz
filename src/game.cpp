#include "game.hpp"

#include "collision.hpp"
#include "objects/circle.hpp"
#include "objects/cube.hpp"
#include "objects/lightcube.hpp"
#include "objects/lightdirectional.hpp"
#include "objects/model.hpp"
#include "objects/player.hpp"
#include "objects/quad.hpp"
#include "objects/skybox.hpp"
#include "resources/builder.hpp"
#include "resources/uniforms/camera.hpp"
#include "resources/uniforms/time.hpp"
#include "resources/uniforms/window.hpp"
#include <imgui.h>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/framebuffer.hpp>
#include <physbuzz/render/gl/capabilities.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/render/uniforms.hpp>
#include <physbuzz/window/bindings.hpp>
#include <random>

Game::Game()
    : builder(&scene) {}

void Game::build() {
    Physbuzz::Context::set(this);
    Physbuzz::Logger::build();

    window.build({1280, 720});
    interface.build(window);

    ResourceBuilder resources;
    resources.build();

    // notify resources and cameras when the window resizes
    window.addCallback<Physbuzz::WindowResizeEvent>([&](const Physbuzz::WindowResizeEvent &event) {
        Physbuzz::Resource<Physbuzz::UniformBuffer<UniformWindow>>("window")->update({
            .resolution = event.resolution,
        });

        for (const auto &[camera] : scene.getComponents<Physbuzz::CameraComponent>()) {
            camera.resize(event.resolution);
        }

        scene.getSystem<Physbuzz::Renderer>()->resize(event.resolution);
        scene.getSystem<Physbuzz::Shadow>()->resize(event.resolution);
    });

    // track cursor captures
    window.addCallback<Physbuzz::MousePositionEvent>([&](const Physbuzz::MousePositionEvent &event) {
        static glm::vec2 lastPosition = event.window->getResolution() >> 1;

        for (const auto &[player, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
            glm::vec2 offset = (static_cast<glm::vec2>(event.position) - lastPosition) * player.sensitivity;
            lastPosition = event.position;

            if (player.captureMouse || interface.draw) {
                return;
            }

            window.setCursorCapture(true);

            const Physbuzz::CameraInfo &info = camera.getInfo();
            glm::quat pitch = glm::angleAxis(glm::radians(offset.x), glm::vec3(0.0f, -1.0f, 0.0f));
            glm::quat yaw = glm::angleAxis(glm::radians(offset.y), glm::cross(camera.getUp(), camera.getFacing()));

            camera.setOrientation(pitch * yaw * info.view.orientation);
        }
    });

    // change prespective camera fov when scrolling
    window.addCallback<Physbuzz::MouseScrollEvent>([&](const Physbuzz::MouseScrollEvent &event) {
        for (const auto &[player, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
            Physbuzz::CameraInfo info = camera.getInfo();

            if (player.captureMouse || ImGui::GetIO().WantCaptureMouse || info.projection != Physbuzz::CameraInfo::Projection::Perspective) {
                return;
            }

            info.perspective.fovy = glm::clamp(info.perspective.fovy + glm::radians<float>(event.offset.y), glm::radians(30.0f), glm::radians(135.0f));

            camera.update(info);
        }
    });

    // enable backface culling and depth testing
    Physbuzz::GL::setCapability(Physbuzz::GL::Capabilities::CullFace, true);
    Physbuzz::GL::setCapability(Physbuzz::GL::Capabilities::DepthTest, true);

    // Create a default scene
    rebuild();
}

void Game::rebuild() {
    Physbuzz::CameraComponent restoreCamera = {{}};
    for (const auto &[_, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
        restoreCamera = camera;
    }

    scene.clear();

    // ticking systems
    {
        scene.createSystem<Collision>(&scene, 0.9);
        scene.createSystem<Physbuzz::Dynamics>(0.0005);
        scene.createSystem<Physbuzz::Bindings>(&window);
        scene.createSystem<Physbuzz::Clock>();
        scene.createSystem<Physbuzz::Shadow>(Physbuzz::ShadowInfo{
            .resolution = window.getResolution(),
            .orthoSize = 1000.0f,
            .depth = 10000.0f,
        });
        scene.createSystem<Physbuzz::Renderer>(Physbuzz::RendererInfo{
            .resolution = window.getResolution(),
            .postProcessing = {
                {"gamma"},
            },
        });
    }

    std::random_device rd;
    std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(-100, 100);

    // player
    {
        Player player = {
            .camera = {{
                .projection = Physbuzz::CameraInfo::Projection::Perspective,
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

        if (restoreCamera.getInfo().projection != Physbuzz::CameraInfo::Projection::Unknown) {
            player.camera = {restoreCamera};
        }

        builder.create(player);
    }

    // skybox
    {
        Skybox skybox;
        builder.create(skybox);
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

        builder.create(backpack);
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

        builder.create(quad);
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

            builder.create(cube);
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

            builder.create(lightCube);
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

        builder.create(directional);
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

        builder.create(point);
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

        for (const auto &[_, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
            Physbuzz::Resource<Physbuzz::UniformBuffer<UniformCamera>>("camera")->update({
                .position = camera.getInfo().view.position,
                ._padding0 = 0.0f,
                .view = camera.getView(),
                .projection = camera.getProjection(),
            });
        }

        // scene.tickSystem<Physbuzz::Dynamics, Collision>();
        scene.tickSystem<Physbuzz::Bindings>();
        scene.tickSystem<Physbuzz::Clock>();
        scene.tickSystem<Physbuzz::Shadow, Physbuzz::Renderer>();

        interface.render();
        window.flip();
    }
}

void Game::destroy() {
    ResourceBuilder resources;
    resources.destroy();

    Physbuzz::ResourceRegistry<Physbuzz::Model>::clear(); // clean up generated models

    m_IsRunning = false;

    interface.destroy();
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
