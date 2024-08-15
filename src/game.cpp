#include "game.hpp"

#include "collision.hpp"
#include "objects/wall.hpp"
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

    glm::ivec2 resolution = glm::ivec2(1920, 1080);

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

    resources.insert<Physbuzz::ShaderPipelineResource>(
        "default",
        Physbuzz::ShaderPipelineResource({
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/default.frag"}},
        }));

    resources.insert<Physbuzz::ShaderPipelineResource>(
        "quad",
        Physbuzz::ShaderPipelineResource({
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/quad.frag"}},
        }));

    resources.insert<Physbuzz::ShaderPipelineResource>(
        "circle",
        Physbuzz::ShaderPipelineResource({
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/circle.frag"}},
        }));

    resources.insert<Physbuzz::ShaderPipelineResource>(
        "cube",
        Physbuzz::ShaderPipelineResource({
            .vertex = {.file = {.path = "resources/shaders/default.vert"}},
            .fragment = {.file = {.path = "resources/shaders/cube.frag"}},
        }));

    resources.build<Physbuzz::ShaderPipelineResource>();

    resources.insert<Physbuzz::Texture2DResource>(
        "missing",
        Physbuzz::Texture2DResource({
            .image = {.file = {.path = "resources/textures/missing.png"}},
        }));

    resources.insert<Physbuzz::Texture2DResource>(
        "wall",
        Physbuzz::Texture2DResource({
            .image = {.file = {.path = "resources/textures/wall.jpg"}},
        }));

    resources.build<Physbuzz::Texture2DResource>();

    reset();

    // post build stuff
    Wall wall = {
        .wall = {
            .position = {resolution >> 1, 0.0f},
            .width = static_cast<float>(resolution.x),
            .height = static_cast<float>(resolution.y),
            .thickness = 10.0f,
        },
        .isCollidable = true,
        .isRenderable = true,
    };

    builder.create(wall);
}

void Game::reset() {
    scene.clear();

    // ticking systems
    {

        scene.emplaceSystem<Collision>(&scene, 0.9);
        scene.emplaceSystem<Physbuzz::Dynamics>(0.0005);

        std::shared_ptr<Renderer> renderer = scene.emplaceSystem<Renderer>(&window, &resources);
        renderer->activeCamera = &player.camera;
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

    scene.buildSystems();
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
    resources.destroy<Physbuzz::ShaderPipelineResource>();
    resources.destroy<Physbuzz::Texture2DResource>();

    m_IsRunning = false;

    for (auto &mesh : scene.getComponents<Physbuzz::MeshComponent>()) {
        mesh.destroy();
    }

    scene.getSystem<Renderer>()->destroy();
    scene.getSystem<Physbuzz::Dynamics>();

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
