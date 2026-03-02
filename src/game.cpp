#include "game.hpp"

#include "objects/cuboid.hpp"
#include "objects/lightdirectional.hpp"
#include "objects/lightpoint.hpp"
#include "objects/model.hpp"
#include "objects/player.hpp"
#include "objects/quad.hpp"
#include "ui/handler.hpp"
#include <filesystem>
#include <physbuzz/app/application.hpp>
#include <physbuzz/compat/imgui/imgui_impl_physbuzz.hpp>
#include <physbuzz/events/window.hpp>
#include <physbuzz/graphics/layout.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/graphics/pipeline.hpp>
#include <physbuzz/graphics/renderer.hpp>
#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/misc/clock.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/deferred.hpp>
#include <physbuzz/render/forward.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/render/skybox.hpp>
#include <physbuzz/window/inputs.hpp>
#include <random>

void Game::build() {
    Physbuzz::App::init();
    Physbuzz::Context::set(this);
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::setResourceDirectory(std::filesystem::current_path() / "resources" / "shaders");
    Physbuzz::ResourceRegistry<Physbuzz::RenderPipeline>::watch();

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
        flashlight.setDirection(camera.getFacing());
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
            .flashlight = {{
                .intensity = {1000.0f, 1000.0f, 1000.0f},
            }},
        };

        playerId = ObjectBuilder::create(Physbuzz::App::GScene, player);
    }

    std::shared_ptr<Physbuzz::Transfer> transfer = Physbuzz::App::GScene.createSystem<Physbuzz::Transfer>();

    std::shared_ptr<Physbuzz::Renderer> renderer = Physbuzz::App::GScene.createSystem<Physbuzz::Renderer>(
        Physbuzz::Renderer::Info{
            .window = window,
        },
        Physbuzz::RenderGraph{{}});

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "skybox",
        {{.type = Physbuzz::Texture::Type::Cube, .sampler = {{Physbuzz::Sampler::Type::Linear}}}},
        std::vector{
            Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/skybox/right.jpg"}},
            Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/skybox/left.jpg"}},
            Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/skybox/top.jpg"}},
            Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/skybox/bottom.jpg"}},
            Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/skybox/front.jpg"}},
            Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/skybox/back.jpg"}},
        },
        transfer);

    std::shared_ptr<Physbuzz::ShadowRenderer> shadow = Physbuzz::App::GScene.createSystem<Physbuzz::ShadowRenderer>(Physbuzz::ShadowRenderer::Info{
        .resolution = {2048, 2048},
    });

    std::shared_ptr<Physbuzz::ForwardRenderer> forward = Physbuzz::App::GScene.createSystem<Physbuzz::ForwardRenderer>(Physbuzz::ForwardRenderer::Info{
        .camera = playerId,
        .window = window,
    });

    std::shared_ptr<Physbuzz::DeferredRenderer> deferred = Physbuzz::App::GScene.createSystem<Physbuzz::DeferredRenderer>(Physbuzz::DeferredRenderer::Info{
        .camera = playerId,
        .window = window,
    });

    std::shared_ptr<Physbuzz::ImGuiRenderer> imgui = Physbuzz::App::GScene.createSystem<Physbuzz::ImGuiRenderer>(Physbuzz::ImGuiRenderer::Info{
        .window = window,
    });

    std::shared_ptr<Physbuzz::SkyboxRenderer> skybox = Physbuzz::App::GScene.createSystem<Physbuzz::SkyboxRenderer>(Physbuzz::SkyboxRenderer::Info{
        .camera = playerId,
        .window = window,
        .skybox = {"skybox"},
    });

    Physbuzz::RenderGraph graph = {{}};

    graph.merge(shadow->getGraph());
    graph.merge(deferred->getGraph());
    graph.merge(skybox->getGraph());
    graph.merge(imgui->getGraph());

    if (!graph.compile()) {
        Physbuzz::Logger::CRITICAL("[Game] Failed to merge render graphs.");
    }

    renderer->setGraph(graph);

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "default/diffuse",
        {{.type = Physbuzz::Texture::Type::Dim2D, .sampler = {{Physbuzz::Sampler::Type::Linear}}}},
        std::vector{Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/default/diffuse.png"}}},
        transfer);

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "default/specular",
        {{.type = Physbuzz::Texture::Type::Dim2D, .sampler = {{Physbuzz::Sampler::Type::Linear}}}},
        std::vector{Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/default/specular.png"}}},
        transfer);

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "floor",
        {{.type = Physbuzz::Texture::Type::Dim2D, .sampler = {{Physbuzz::Sampler::Type::Linear}}}},
        std::vector{Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/floor.png"}}},
        transfer);

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "crate/diffuse",
        {{.type = Physbuzz::Texture::Type::Dim2D, .sampler = {{Physbuzz::Sampler::Type::Linear}}}},
        std::vector{Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/crate/diffuse.png"}}},
        transfer);

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        "crate/specular",
        {{.type = Physbuzz::Texture::Type::Dim2D, .sampler = {{Physbuzz::Sampler::Type::Linear}}}},
        std::vector{Physbuzz::ImageFile::Info{.file = {.path = "resources/textures/crate/specular.png"}}},
        transfer);

    Physbuzz::ResourceRegistry<Physbuzz::Material>::insert(
        "default",
        {
            .shininess = 256.0f,
            .textures = {
                {
                    Physbuzz::TextureType::Diffuse,
                    {
                        {"default/diffuse"},
                    },
                },
                {
                    Physbuzz::TextureType::Specular,
                    {
                        {"default/specular"},
                    },
                },
            },
        });

    Physbuzz::ResourceRegistry<Physbuzz::Material>::insert(
        "crate",
        {
            .shininess = 256.0f,
            .textures = {
                {
                    Physbuzz::TextureType::Diffuse,
                    {
                        {"crate/diffuse"},
                    },
                },
                {
                    Physbuzz::TextureType::Specular,
                    {
                        {"crate/specular"},
                    },
                },
            },
        });

    Physbuzz::ResourceRegistry<Physbuzz::Material>::insert(
        "floor",
        {
            .shininess = 256.0f,
            .textures = {
                {
                    Physbuzz::TextureType::Diffuse,
                    {
                        {"floor"},
                    },
                },
            },
        });

    Physbuzz::App::GScene.createSystem<Physbuzz::InputEvents>(window);
    Physbuzz::App::GScene.createSystem<Physbuzz::Clock>();
    Physbuzz::App::GScene.createSystem<Physbuzz::Dynamics>(1.0f);
    Physbuzz::App::GScene.createSystem<InterfaceManager>();

    std::random_device rd;
    std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(-250, 250);

    // cubes
    {
        for (int i = 0; i < 5; ++i) {
            Cuboid cube = {
                .cuboid = {
                    .width = 50.0f,
                    .breadth = 50.0f,
                    .height = 50.0f,
                },
                .transform = {{
                    .position = {(i - 5 / 2) * 100, 0, 0},
                }},
                .resources = {
                    .material = {"crate"},
                },
                .hasPhysics = false,
            };

            ObjectBuilder::create(Physbuzz::App::GScene, cube);
        }
    }

    {
        Quad quad = {
            .body = {},
            .quad = {
                .width = 1500.0f,
                .height = 1500.0f,
            },
            .transform = {{
                .position = {0.0f, -300.0f, 0.0f},
                .orientation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            }},
            .resources = {
                .material = {"floor"},
            },
        };

        ObjectBuilder::create(Physbuzz::App::GScene, quad);
    }

    // point lights
    {
        for (int i = 0; i < 1; ++i) {
            LightPoint point = {
                .sphere = {
                    .radius = 10.0f,
                },
                .transform = {{
                    .position = {(i - 5 / 2) * 100, 150, 0},
                    .orientation = glm::angleAxis(glm::radians(static_cast<float>(distribution(rd) % 360)), glm::normalize(glm::vec3(distribution(rd), distribution(rd), distribution(rd)))),
                }},
                .pointLight = {{
                    .intensity = {10000.0f, 10000.0f, 10000.0f},
                    .depth = 2000.0f,
                }},
            };

            ObjectBuilder::create(Physbuzz::App::GScene, point);
        }
    }

    {
        LightDirectional directional = {
            .directionalLight = {{
                .direction = glm::normalize(glm::vec3{1.0f, -1.0f, -1.0f}),
                .intensity = {1.0f, 1.0f, 1.0f},
                .orthoSize = 1000.0f,
                .depth = 2200.0f,
            }},
        };

        ObjectBuilder::create(Physbuzz::App::GScene, directional);
    }

    {
        Model model = {
            .model = {
                .path = "resources/models/backpack/backpack.obj",
            },
            .transform = {{
                .position = {0, -150, 0},
                .scale = {30, 30, 30},
            }},
            .identifier = {
                .name = "Backpack",
            },
        };

        ObjectBuilder::create(Physbuzz::App::GScene, model);
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
        Physbuzz::App::GScene.tickSystem<Physbuzz::InputEvents>();
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
