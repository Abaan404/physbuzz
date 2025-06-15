#include "game.hpp"

#include "collision.hpp"
#include "objects/circle.hpp"
#include "objects/cube.hpp"
#include "objects/lightcube.hpp"
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
#include <physbuzz/render/gl/capabilities.hpp>
#include <physbuzz/render/uniforms.hpp>
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
        Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformWindow>>("window")->update({
            .resolution = event.resolution,
        });

        for (const auto &[camera] : scene.getComponents<Physbuzz::CameraComponent>()) {
            camera.resize(event.resolution);
        }

        scene.getSystem<Physbuzz::Renderer>()->resize(event.resolution);
    });

    // track cursor captures
    window.addCallback<Physbuzz::MousePositionEvent>([&](const Physbuzz::MousePositionEvent &event) {
        static glm::vec2 lastPosition = event.window->getResolution() >> 1;

        for (const auto &[player, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
            glm::vec2 offset = (static_cast<glm::vec2>(event.position) - lastPosition) * player.sensitivity;
            lastPosition = event.position;

            const Physbuzz::CameraInfo &cameraInfo = camera.getInfo();

            if (player.captureMouse || interface.draw || cameraInfo.type == Physbuzz::CameraInfo::Type::Orthographic2D) {
                return;
            }

            window.setCursorCapture(true);

            glm::quat pitch = glm::angleAxis(glm::radians(offset.x), glm::vec3(0.0f, -1.0f, 0.0f));
            glm::quat yaw = glm::angleAxis(glm::radians(offset.y), glm::cross(camera.getUp(), camera.getFacing()));

            camera.setOrientation(pitch * yaw * cameraInfo.view.orientation);
        }
    });

    // change prespective camera fov when scrolling
    window.addCallback<Physbuzz::MouseScrollEvent>([&](const Physbuzz::MouseScrollEvent &event) {
        for (const auto &[player, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
            const Physbuzz::CameraInfo &cameraInfo = camera.getInfo();

            if (player.captureMouse || ImGui::GetIO().WantCaptureMouse || cameraInfo.type != Physbuzz::CameraInfo::Type::Prespective) {
                return;
            }

            Physbuzz::CameraInfo::Prespective prespective = cameraInfo.prespective;
            prespective.fovy = glm::clamp(prespective.fovy + glm::radians<float>(event.offset.y), glm::radians(30.0f), glm::radians(135.0f));

            camera.setPrespective(prespective);
        }
    });

    // enable backface culling
    Physbuzz::GL::setCapability(Physbuzz::GL::Capabilities::CullFace, true);

    // Create a default scene
    rebuild();
}

void Game::rebuild() {
    scene.clear();

    // ticking systems
    {
        scene.createSystem<Collision>(&scene, 0.9);
        scene.createSystem<Physbuzz::Dynamics>(0.0005);
        scene.createSystem<Physbuzz::Bindings>(&window);
        scene.createSystem<Physbuzz::Clock>();
        scene.createSystem<Physbuzz::Renderer>(Physbuzz::RendererInfo{
            .framebuffer = {
                .resolution = window.getResolution(),
                .colorClear = {0.0f, 0.0f, 0.0f, 0.0f},
            },
        });
    }

    std::random_device rd;
    std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(-250, 250);

    // player
    {
        Player player = {
            .camera = {{
                .type = Physbuzz::CameraInfo::Type::Prespective,
                .orthographic = {},
                .prespective = {
                    .fovy = glm::radians(45.0f),
                },
                .depth = {
                    .near = 1.0f,
                    .far = 10000.0f,
                },
                .view = {
                    .position = {0.0f, 50.0f, 0.0f},
                },
                .resolution = {window.getResolution()},
            }},
        };
        Physbuzz::GL::setCapability(Physbuzz::GL::Capabilities::DepthTest, true);

        builder.create(player);
    }

    // skybox
    {
        Skybox skybox = {
            .skybox = {},
            .transform = {},
            .resources = {
                .renderpasses = {
                    {"skybox"},
                },
            },
        };

        builder.create(skybox);
    }

    // backpack
    {
        Model backpack = {
            .body = {},
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
                .width = 500.0f,
                .height = 500.0f,
            },
            .transform = {
                .orientation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            },
            .resources = {
                .textures = {
                    {"floor"},
                    {"default/specular"},
                },
                .renderpasses = {{"default"}},
            },
        };

        builder.create(quad);
    }

    // cubes
    {
        for (int i = 0; i < 25; ++i) {
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
                    .renderpasses = {
                        {"default"},
                    },
                },
                .hasPhysics = false,
            };

            builder.create(cube);
        }
    }

    // point lights
    {
        for (int i = 0; i < 3; ++i) {
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
                .renderpasses = {
                    {"circle"},
                },
            },
        };

        builder.create(point);
    }
}

void Game::loop() {
    m_IsRunning = true;

    while (m_IsRunning && !window.shouldClose()) {
        const std::shared_ptr<Physbuzz::Clock> clock = scene.getSystem<Physbuzz::Clock>();
        Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformTime>>("time")->update({
            .time = clock->getTime(),
            .timedelta = clock->getDelta(),
        });

        for (const auto &[player, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
            Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->update({
                .position = camera.getInfo().view.position,
                ._padding0 = {},
                .view = camera.getView(),
                .projection = camera.getProjection(),
            });
        }

        // scene.tickSystem<Physbuzz::Dynamics, Collision>();
        scene.tickSystem<Physbuzz::Bindings>();
        scene.tickSystem<Physbuzz::Clock>();
        scene.tickSystem<Physbuzz::Renderer>();

        interface.render();
        window.flip();
    }
}

void Game::destroy() {
    ResourceBuilder resources;
    resources.destroy();

    Physbuzz::ResourceRegistry<Physbuzz::ModelResource>::clear(); // clean up generated models

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
