#include "game.hpp"

#include "collision.hpp"
#include "objects/circle.hpp"
#include "objects/cube.hpp"
#include "objects/lightcube.hpp"
#include "objects/model.hpp"
#include "objects/quad.hpp"
#include "objects/skybox.hpp"
#include "renderer.hpp"
#include "resources/builder.hpp"
#include <glm/ext/vector_float3.hpp>
#include <physbuzz/events/scene.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/mesh.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <random>

Game::Game()
    : bindings(&window), player(this), builder(&scene) {}

Game::~Game() {}

void Game::build() {
    Physbuzz::Context::set(this);
    Physbuzz::Logger::build();

    glm::ivec2 resolution = glm::ivec2(1280, 720);

    window.build(resolution);
    player.build();
    bindings.build();
    interface.build(window);

    ResourceBuilder builder;
    builder.buildTextures();
    builder.buildModels();
    builder.buildCubemaps();
    builder.buildUniforms();
    builder.buildShaders();

    window.addCallback<Physbuzz::WindowResizeEvent>([&](const Physbuzz::WindowResizeEvent &event) {
        player.resize(event.resolution);
        scene.getSystem<Renderer>()->resize(event.resolution);
    });

    window.addCallback<Physbuzz::WindowCloseEvent>([&](const Physbuzz::WindowCloseEvent &event) {
        window.close();
    });

    // Create a default scene
    rebuild();
}

void Game::rebuild() {
    scene.clear();

    // ticking systems
    {
        scene.emplaceSystem<Collision>(&scene, 0.9);
        scene.emplaceSystem<Physbuzz::Dynamics>(0.0005);

        std::shared_ptr<Renderer> renderer = scene.emplaceSystem<Renderer>(&window);
        renderer->activeCamera = &player.camera;

        scene.buildSystems();
    }

    // callbacks for cleaning up meshes
    {
        scene.addCallback<Physbuzz::OnComponentEraseEvent<Physbuzz::Mesh>>([](const Physbuzz::OnComponentEraseEvent<Physbuzz::Mesh> &event) {
            event.component->destroy();
        });

        scene.addCallback<Physbuzz::OnObjectEraseEvent>([](const Physbuzz::OnObjectEraseEvent &event) {
            if (event.scene->containsComponent<Physbuzz::Mesh>(event.object)) {
                event.scene->getComponent<Physbuzz::Mesh>(event.object).destroy();
            }
        });

        scene.addCallback<Physbuzz::OnSceneClear>([](const Physbuzz::OnSceneClear &event) {
            for (const auto id : event.scene->getObjects()) {
                if (event.scene->containsComponent<Physbuzz::Mesh>(id)) {
                    event.scene->getComponent<Physbuzz::Mesh>(id).destroy();
                }
            }
        });
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
                .resource = "backpack",
            },
            .transform = {
                .position = glm::vec3(0.0f, 50.0f, 0.0f),
                .scale = glm::vec3(20.0f, 20.0f, 20.0f),
                .orientation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            },
        };

        builder.create(backpack);
    }

    // platform
    {
        Quad quad = {
            .quad = {
                .width = 500.0f,
                .height = 500.0f,
            },
            .transform = {
                .orientation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            },
            .resources = {
                .textures = {
                    {Physbuzz::TextureType::Diffuse, {"wall"}},
                    {Physbuzz::TextureType::Specular, {"default/specular"}},
                },
                .pipeline = "default",
            },
        };

        builder.create(quad);
    }

    // illuminating pointlight cubes
    {
        std::random_device rd;
        std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(-250, 250);

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
                        {Physbuzz::TextureType::Diffuse, {"crate/diffuse"}},
                        {Physbuzz::TextureType::Specular, {"crate/specular"}},
                    },
                    .pipeline = "default",
                },
                .hasPhysics = false,
            };

            builder.create(cube);
        }

        // point lights
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
                    .identifier = {
                        .name = "LightCube",
                    },
                    .resources = {
                        .textures = {
                            {Physbuzz::TextureType::Diffuse, {"default/specular"}},
                            {Physbuzz::TextureType::Specular, {"default/specular"}},
                        },
                        .pipeline = "default",
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

        // a circle because why not?
        Circle point = {
            .circle = {
                .radius = 10.0f,
            },
            .transform = {
                .position = {100.0f, 100.0f, 100.0f},
            },
            .resources = {
                .textures = {
                    {Physbuzz::TextureType::Diffuse, {"default/diffuse"}},
                    {Physbuzz::TextureType::Specular, {"default/specular"}},
                },
                .pipeline = "circle",
            },
        };

        builder.create(point);
    }
}

void Game::loop() {
    m_IsRunning = true;

    while (m_IsRunning && !window.shouldClose()) {
        bindings.poll();

        // scene.tickSystem<Physbuzz::Dynamics, Collision>();
        scene.tickSystem<Renderer>();

        interface.render();
        window.flip();
    }
}

void Game::destroy() {
    Physbuzz::ResourceRegistry::clear<Physbuzz::ShaderPipelineResource>();
    Physbuzz::ResourceRegistry::clear<Physbuzz::Texture2DResource>();

    m_IsRunning = false;

    for (const auto &object : scene.getObjects()) {
        if (scene.containsComponent<Physbuzz::Mesh>(object)) {
            scene.getComponent<Physbuzz::Mesh>(object).destroy();
        }
    }

    interface.destroy();
    bindings.destory();
    scene.clear();
    player.destroy();
    window.destroy();
}

int main(int argc, char *argv[]) {
    Game game = Game();

    game.build();
    game.loop();
    game.destroy();

    return 0;
}
