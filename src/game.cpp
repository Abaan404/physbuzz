#include "game.hpp"

#include "collision.hpp"
#include "objects/lightdirectional.hpp"
#include "objects/lightpoint.hpp"
#include "objects/materialshape.hpp"
#include "objects/model.hpp"
#include "objects/player.hpp"
#include "objects/shape.hpp"
#include "renderers/collision_highlight.hpp"
#include "resources/loader.hpp"
#include "shapes/circle.hpp"
#include "shapes/cube.hpp"
#include "shapes/sphere.hpp"
#include "shapes/square.hpp"
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
    Physbuzz::ResourceRegistry<Physbuzz::GraphicsPipeline>::setResourceDirectory(std::filesystem::current_path() / "resources" / "shaders");
    Physbuzz::ResourceRegistry<Physbuzz::ComputePipeline>::setResourceDirectory(std::filesystem::current_path() / "resources" / "shaders");

    Physbuzz::ResourceRegistry<Physbuzz::GraphicsPipeline>::watch();
    Physbuzz::ResourceRegistry<Physbuzz::ComputePipeline>::watch();

    std::shared_ptr<Physbuzz::Window> window = Physbuzz::App::createWindow("main", {}, {1280, 720});

    // track cursor captures
    window->addCallback<Physbuzz::MousePositionEvent>([](const Physbuzz::MousePositionEvent &event) {
        static glm::vec2 lastPosition = event.window->getResolution() >> 1u;

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

    Physbuzz::Transfer *transfer = Physbuzz::App::GScene.createSystem<Physbuzz::Transfer>(Physbuzz::Transfer::Info{});

    Physbuzz::Renderer *renderer = Physbuzz::App::GScene.createSystem<Physbuzz::Renderer>(
        Physbuzz::Renderer::Info{
            .window = window,
        },
        Physbuzz::RenderGraph{{}});

    Physbuzz::TransferBatch batch = {{}};

    Physbuzz::Geometry<Physbuzz::Model::Vertex> cube = ModelCube::build();
    Physbuzz::Geometry<Physbuzz::Model::Vertex> square = ModelSquare::build();
    Physbuzz::Geometry<Physbuzz::Model::Vertex> circle = ModelCircle::build();
    Physbuzz::Geometry<Physbuzz::Model::Vertex> sphere = ModelSphere::build();

    Physbuzz::Resource<Physbuzz::Mesh> cubeMesh = *ResourceLoader::loadMesh({"cube"}, cube, batch);
    Physbuzz::Resource<Physbuzz::Mesh> squareMesh = *ResourceLoader::loadMesh({"square"}, square, batch);
    Physbuzz::Resource<Physbuzz::Mesh> circleMesh = *ResourceLoader::loadMesh({"circle"}, circle, batch);
    Physbuzz::Resource<Physbuzz::Mesh> sphereMesh = *ResourceLoader::loadMesh({"sphere"}, sphere, batch);

    Physbuzz::Resource<Physbuzz::Mesh> cubeShadowMesh = *ResourceLoader::loadShadowMesh({"cube/shadow"}, cube, batch);
    Physbuzz::Resource<Physbuzz::Mesh> squareShadowMesh = *ResourceLoader::loadShadowMesh({"square/shadow"}, square, batch);
    Physbuzz::Resource<Physbuzz::Mesh> circleShadowMesh = *ResourceLoader::loadShadowMesh({"circle/shadow"}, circle, batch);
    Physbuzz::Resource<Physbuzz::Mesh> sphereShadowMesh = *ResourceLoader::loadShadowMesh({"sphere/shadow"}, sphere, batch);

    Physbuzz::Resource<Physbuzz::PhysicsMesh> cubePhysicsMesh = *ResourceLoader::loadPhysicsMesh({"cube"}, cube, [](const glm::vec3 &direction) {
        return glm::vec3(0.5f) * glm::sign(direction);
    });

    Physbuzz::Resource<Physbuzz::PhysicsMesh> squarePhysicsMesh = *ResourceLoader::loadPhysicsMesh({"square"}, square, [](const glm::vec3 &direction) {
        return glm::vec3(0.5f, 0.5f, 0.0f) * glm::sign(direction);
    });

    Physbuzz::Resource<Physbuzz::PhysicsMesh> circlePhysicsMesh = *ResourceLoader::loadPhysicsMesh({"circle"}, circle, [](const glm::vec3 &direction) {
        return glm::vec3(glm::normalize(direction));
    });

    Physbuzz::Resource<Physbuzz::PhysicsMesh> spherePhysicsMesh = *ResourceLoader::loadPhysicsMesh({"sphere"}, sphere, [](const glm::vec3 &direction) {
        return glm::normalize(direction);
    });

    Physbuzz::Resource<Physbuzz::Texture> albedoDefault = *ResourceLoader::loadTexture("default/albedo", {.files = {{.path = "resources/textures/default/albedo.png"}}}, batch);
    Physbuzz::Resource<Physbuzz::Texture> metallicDefault = *ResourceLoader::loadTexture("default/metallic", {.files = {{.path = "resources/textures/default/metallic.png"}}}, batch);

    Physbuzz::Resource<Physbuzz::Texture> albedoCrate = *ResourceLoader::loadTexture("crate/albedo", {.files = {{.path = "resources/textures/crate/albedo.png"}}}, batch);
    Physbuzz::Resource<Physbuzz::Texture> metallicCrate = *ResourceLoader::loadTexture("crate/metallic", {.files = {{.path = "resources/textures/crate/metallic.png"}}}, batch);

    Physbuzz::Resource<Physbuzz::Texture> albedoFloor = *ResourceLoader::loadTexture("floor", {.files = {{.path = "resources/textures/floor.png"}}}, batch);

    Physbuzz::Resource<Physbuzz::Material> defaultMaterial = *Physbuzz::ResourceRegistry<Physbuzz::Material>::insert(
        "default",
        {
            .textures = {
                {Physbuzz::TextureType::Albedo, std::move(albedoDefault)},
                {Physbuzz::TextureType::Metallic, std::move(metallicDefault)},
            },
        });

    Physbuzz::Resource<Physbuzz::Material> crateMaterial = *Physbuzz::ResourceRegistry<Physbuzz::Material>::insert(
        "crate",
        {
            .textures = {
                {Physbuzz::TextureType::Albedo, std::move(albedoCrate)},
                {Physbuzz::TextureType::Metallic, std::move(metallicCrate)},
            },
        });

    Physbuzz::Resource<Physbuzz::Material> floorMaterial = *Physbuzz::ResourceRegistry<Physbuzz::Material>::insert(
        "floor",
        {
            .textures = {
                {Physbuzz::TextureType::Albedo, std::move(albedoFloor)},
            },
        });

    {
        std::vector<std::string> materials = {"gold", "grass", "plastic", "rusted_iron", "wall"};

        for (int i = 0; i < materials.size(); i++) {
            const auto &materialName = materials[i];

            Physbuzz::Resource<Physbuzz::Texture> albedo = *ResourceLoader::loadTexture(
                std::format("{}/albedo.png", materialName),
                {.files = {{.path = std::format("resources/textures/{}/albedo.png", materialName)}}},
                batch);

            Physbuzz::Resource<Physbuzz::Texture> ao = *ResourceLoader::loadTexture(
                std::format("{}/ao.png", materialName),
                {.files = {{.path = std::format("resources/textures/{}/ao.png", materialName)}}},
                batch);

            Physbuzz::Resource<Physbuzz::Texture> metallic = *ResourceLoader::loadTexture(
                std::format("{}/metallic.png", materialName),
                {.files = {{.path = std::format("resources/textures/{}/metallic.png", materialName)}}},
                batch);

            Physbuzz::Resource<Physbuzz::Texture> normal = *ResourceLoader::loadTexture(
                std::format("{}/normal.png", materialName),
                {.files = {{.path = std::format("resources/textures/{}/normal.png", materialName)}}},
                batch);

            Physbuzz::Resource<Physbuzz::Texture> roughness = *ResourceLoader::loadTexture(
                std::format("{}/roughness.png", materialName),
                {.files = {{.path = std::format("resources/textures/{}/roughness.png", materialName)}}},
                batch);

            Physbuzz::Resource<Physbuzz::Material> material = *Physbuzz::ResourceRegistry<Physbuzz::Material>::insert(
                materialName,
                {
                    .textures = {
                        {Physbuzz::TextureType::Albedo, std::move(albedo)},
                        {Physbuzz::TextureType::AmbientOcclusion, std::move(ao)},
                        {Physbuzz::TextureType::Metallic, std::move(metallic)},
                        {Physbuzz::TextureType::Normal, std::move(normal)},
                        {Physbuzz::TextureType::Roughness, std::move(roughness)},
                    },
                });

            MaterialShape sphere = {
                .transform = {{
                    .position = {(i - 2) * 50.0f, 100.0f, 0},
                    .scale = {10.0f, 10.0f, 10.0f},
                }},
                .resources = {
                    .mesh = sphereMesh,
                    .material = material,
                },
            };

            ObjectBuilder::create(Physbuzz::App::GScene, sphere);
        }
    }

    Physbuzz::Resource<Physbuzz::Texture> hdrSkyboxTexture = *ResourceLoader::loadTexture("hdr_skybox", {.files = {{.path = "resources/textures/hdr_skybox/newport_loft.hdr"}}, .flipVertically = true}, batch);

    Physbuzz::Resource<Physbuzz::Texture> skyboxTexture = *ResourceLoader::loadCubemap(
        "skybox",
        {
            .files = {
                {.path = "resources/textures/skybox/right.jpg"},
                {.path = "resources/textures/skybox/left.jpg"},
                {.path = "resources/textures/skybox/top.jpg"},
                {.path = "resources/textures/skybox/bottom.jpg"},
                {.path = "resources/textures/skybox/front.jpg"},
                {.path = "resources/textures/skybox/back.jpg"},
            },
        },
        batch);

    transfer->submit(batch);

    Physbuzz::ResourceContext *resourceContext = Physbuzz::App::GScene.createSystem<Physbuzz::ResourceContext>();
    resourceContext->setCamera(playerId);

    Physbuzz::ShadowRenderer *shadow = Physbuzz::App::GScene.createSystem<Physbuzz::ShadowRenderer>(
        Physbuzz::ShadowRenderer::Info{
            .resolution = {2048, 2048},
        });

    Physbuzz::ForwardRenderer *forward = Physbuzz::App::GScene.createSystem<Physbuzz::ForwardRenderer>(
        Physbuzz::ForwardRenderer::Info{
            .window = window,
            .environmentMap = hdrSkyboxTexture,
        });

    Physbuzz::DeferredRenderer *deferred = Physbuzz::App::GScene.createSystem<Physbuzz::DeferredRenderer>(
        Physbuzz::DeferredRenderer::Info{
            .window = window,
        });

    Physbuzz::ImGuiRenderer *imgui = Physbuzz::App::GScene.createSystem<Physbuzz::ImGuiRenderer>(
        Physbuzz::ImGuiRenderer::Info{
            .window = window,
        });

    Physbuzz::SkyboxRenderer *skybox = Physbuzz::App::GScene.createSystem<Physbuzz::SkyboxRenderer>(
        Physbuzz::SkyboxRenderer::Info{
            .window = window,
            .skybox = hdrSkyboxTexture,
        });

    CollisionHighlight *collisionHighlight = Physbuzz::App::GScene.createSystem<CollisionHighlight>(
        CollisionHighlight::Info{
            .window = window,
            .objects = forward->getObjects(),
            .batch = forward->getBatch(),
        });

    Physbuzz::RenderGraph graph = {{}};

    graph.merge(forward->getGraph());
    graph.merge(skybox->getGraph());
    graph.merge(imgui->getGraph());
    graph.merge(collisionHighlight->getGraph());

    if (!graph.compile()) {
        Physbuzz::Logger::CRITICAL("[Game] Failed to merge render graphs.");
    }

    renderer->setGraph(graph);

    Physbuzz::App::GScene.createSystem<Collision>(
        Collision::Info{
            .restitution = 1.0f,
        });

    Physbuzz::App::GScene.createSystem<Physbuzz::InputEvents>(window);
    Physbuzz::App::GScene.createSystem<Physbuzz::Clock>();
    Physbuzz::App::GScene.createSystem<InterfaceManager>();

    std::random_device rd;
    std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(-250, 250);

    // cubes
    {
        float density = 1.0f;
        float length = 50.0f;

        for (int i = 0; i < 5; ++i) {
            Shape shape = {
                .transform = {{
                    .position = {(i - 5 / 2) * 100, 10.0f, 0},
                    .scale = glm::vec3(length),
                }},
                .body = {{
                    .mesh = cubePhysicsMesh,
                    .inverseMass = 1.0f,
                    .angular = {
                        .inverseInertia = glm::inverse(glm::mat3(1.0f / 6.0f * density * length * length)),
                    },
                }},
                .identifier = {
                    .name = "Cuboid",
                    .hidden = false,
                },
                .resources = {
                    .mesh = cubeMesh,
                    .material = crateMaterial,
                    .shadowMesh = cubeShadowMesh,
                },
                .hasPhysics = true,
                .hasGravity = true,
            };

            ObjectBuilder::create(Physbuzz::App::GScene, shape);
        }
    }

    // spheres
    {
        float density = 1.0f;
        float radius = 10.0f;

        for (int i = 0; i < 5; ++i) {
            Shape shape = {
                .transform = {{
                    .position = {(i - 5 / 2) * 100, 200, 0},
                    .scale = glm::vec3(2.0f * radius),
                }},
                .body = {{
                    .mesh = spherePhysicsMesh,
                    .inverseMass = 1.0f,
                    .angular = {
                        .inverseInertia = glm::inverse(glm::mat3(7.0f / 5.0f * density * radius * radius)),
                    },
                }},
                .identifier = {
                    .name = "Sphere",
                    .hidden = false,
                },
                .resources = {
                    .mesh = sphereMesh,
                    .material = crateMaterial,
                    .shadowMesh = sphereShadowMesh,
                },
                .hasPhysics = true,
                .hasGravity = true,
            };

            ObjectBuilder::create(Physbuzz::App::GScene, shape);
        }
    }

    {
        Shape shape = {
            .transform = {{
                .position = {0.0f, -300.0f, 0.0f},
                .scale = {1500.0f, 1500.0f, 1.0f},
                .orientation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            }},
            .body = {{
                .mesh = squarePhysicsMesh,
            }},
            .identifier = {
                .name = "Floor",
                .hidden = false,
            },
            .resources = {
                .mesh = squareMesh,
                .material = floorMaterial,
            },
            .hasPhysics = true,
        };

        ObjectBuilder::create(Physbuzz::App::GScene, shape);
    }

    // point lights
    {
        for (int i = 0; i < 1; ++i) {
            LightPoint point = {
                .transform = {{
                    .position = {(i - 5 / 2) * 100, 150, 0},
                    .scale = {10.0f, 10.0f, 10.0f},
                    .orientation = glm::angleAxis(glm::radians(static_cast<float>(distribution(rd) % 360)), glm::normalize(glm::vec3(distribution(rd), distribution(rd), distribution(rd)))),
                }},
                .resources = {
                    .mesh = circleMesh,
                    .shadowMesh = circleShadowMesh,
                },
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
                .orthoSize = 3000.0f,
                .depth = 5000.0f,
            }},
        };

        ObjectBuilder::create(Physbuzz::App::GScene, directional);
    }

    {
        Model model = {
            .model = {
                .path = "resources/models/survival_guitar_backpack/scene.gltf",
                .flipUVs = true,
            },
            .transform = {{
                .position = {0, -150.0f, 0},
                .scale = {0.5f, 0.5f, 0.5f},
            }},
            .identifier = {
                .name = "Survival Guitar Backpack",
            },
        };

        ObjectBuilder::create(Physbuzz::App::GScene, model);
    }

    {
        Model model = {
            .model = {
                .path = "resources/models/cerberusffvii_gun_model_by_andrew_maximov/scene.gltf",
                .flipUVs = true,
            },
            .transform = {{
                .position = {0, 200.0f, 0},
                .scale = {1.0f, 1.0f, 1.0f},
            }},
            .identifier = {
                .name = "Cerberus",
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
        Physbuzz::App::GScene.tickSystem<Physbuzz::Dynamics, Collision>();
        Physbuzz::App::GScene.tickSystem<Physbuzz::InputEvents>();
        Physbuzz::App::GScene.tickSystem<Physbuzz::ForwardRenderer, Physbuzz::ShadowRenderer, InterfaceManager, Physbuzz::Renderer>();
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
