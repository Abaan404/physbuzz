#include "game.hpp"
#include "objects/wall.hpp"

#include <physbuzz/shaders.hpp>
#include <physbuzz/2D/detectors/gjk.hpp>
#include <physbuzz/2D/detectors/sweepandprune.hpp>
#include <physbuzz/2D/resolvers/angular.hpp>
#include <physbuzz/context.hpp>

Game::Game()
    : collision(
          std::make_shared<Physbuzz::Gjk2D>(scene),
          std::make_shared<Physbuzz::SweepAndPrune2D>(scene),
          std::make_shared<Physbuzz::AngularResolver2D>(scene, 0.9f)),
      dynamics(0.005),
      bindings(&window),
      player(this),
      renderer(&window),
      builder(scene) {}

Game::~Game() {}

void Game::build() {
    Physbuzz::Context::set(this);

    glm::ivec2 resolution = glm::ivec2(1920, 1080);

    window.build(resolution);
    player.build();
    renderer.build();
    bindings.build();
    interface.build(window);
    collision.build();

    Wall wall = {
        .transform = {
            .position = {resolution >> 1, 0.0f},
        },
        .wall = {
            .width = static_cast<float>(resolution.x),
            .height = static_cast<float>(resolution.y),
            .thickness = 10.0f,
        },
        .isCollidable = true,
        .isRenderable = true,
    };

    // builder.create(wall);

    window.addCallback<Physbuzz::WindowResizeEvent>([&](const Physbuzz::WindowResizeEvent &event) {
        player.resize(event.resolution);
        renderer.resize(event.resolution);
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
}

void Game::loop() {
    m_IsRunning = true;
    renderer.activeCamera = &player.camera;

    while (m_IsRunning && !window.shouldClose()) {
        window.poll();

        // collision.tick(scene);
        // dynamics.tick(scene);
        bindings.tick();

        renderer.render(scene, resources);
        interface.render();

        window.flip();
    }
}

void Game::destroy() {
    resources.destroy<Physbuzz::ShaderPipelineResource>();
    resources.destroy<Physbuzz::Texture2DResource>();

    m_IsRunning = false;

    for (auto &mesh : scene.getComponents<Physbuzz::Mesh>()) {
        mesh.destroy();
    }

    collision.destroy();
    interface.destroy();
    bindings.destory();
    scene.clear();
    renderer.destroy();
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
