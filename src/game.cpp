#include "game.hpp"

#include "collision.hpp"
#include "objects/quad.hpp"
#include "renderer.hpp"
#include <physbuzz/events/scene.hpp>
#include <physbuzz/misc/context.hpp>
#include <physbuzz/render/mesh.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>

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

    window.addCallback<Physbuzz::WindowResizeEvent>([&](const Physbuzz::WindowResizeEvent &event) {
        player.resize(event.resolution);
        scene.getSystem<Renderer>()->resize(event.resolution);
    });

    window.addCallback<Physbuzz::WindowCloseEvent>([&](const Physbuzz::WindowCloseEvent &event) {
        window.close();
    });

    Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>(
        "default",
        {{
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/default.frag"}},
        }});

    Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>(
        "quad",
        {{
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/quad.frag"}},
        }});

    Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>(
        "circle",
        {{
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/circle.frag"}},
        }});

    Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>(
        "cube",
        {{
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/cube.frag"}},
        }});

    Physbuzz::ResourceRegistry::insert<Physbuzz::Texture2DResource>(
        "missing",
        {{
            .image = {.file = {.path = "resources/textures/missing.png"}},
        }});

    Physbuzz::ResourceRegistry::insert<Physbuzz::Texture2DResource>(
        "wall",
        {{
            .image = {.file = {.path = "resources/textures/wall.jpg"}},
        }});

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
        scene.addCallback<Physbuzz::OnComponentEraseEvent<Physbuzz::MeshComponent>>([](const Physbuzz::OnComponentEraseEvent<Physbuzz::MeshComponent> &event) {
            event.component->destroy();
        });

        scene.addCallback<Physbuzz::OnObjectEraseEvent>([](const Physbuzz::OnObjectEraseEvent &event) {
            if (event.scene->containsComponent<Physbuzz::MeshComponent>(event.object)) {
                event.scene->getComponent<Physbuzz::MeshComponent>(event.object).destroy();
            }
        });

        scene.addCallback<Physbuzz::OnSceneClear>([](const Physbuzz::OnSceneClear &event) {
            for (const auto id : event.scene->getObjects()) {
                if (event.scene->containsComponent<Physbuzz::MeshComponent>(id)) {
                    event.scene->getComponent<Physbuzz::MeshComponent>(id).destroy();
                }
            }
        });
    }

    // platform
    {
        Quad quad = {
            .model = {
                .orientation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            },
            .quad = {
                .width = 500.0f,
                .height = 500.0f,
            },
            .resources = {.texture2D = "wall"},
            .isRenderable = true,
        };

        builder.create(quad);
    }
}

void Game::loop() {
    m_IsRunning = true;

    while (m_IsRunning && !window.shouldClose()) {
        bindings.poll();

        scene.tickSystem<Physbuzz::Dynamics, Collision>();
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
        if (scene.containsComponent<Physbuzz::MeshComponent>(object)) {
            scene.getComponent<Physbuzz::MeshComponent>(object).destroy();
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
