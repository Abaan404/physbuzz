#include "game.hpp"

#include "events.hpp"
#include "ui/handler.hpp"
#include <physbuzz/2D/detectors/gjk.hpp>
#include <physbuzz/2D/detectors/sweepandprune.hpp>
#include <physbuzz/2D/resolvers/angular.hpp>
#include <physbuzz/context.hpp>
#include <physbuzz/dynamics.hpp>
#include <physbuzz/events.hpp>
#include <physbuzz/renderer.hpp>

Game::Game()
    : collision(
          std::make_shared<Physbuzz::Gjk2D>(scene),
          std::make_shared<Physbuzz::SweepAndPrune2D>(scene),
          std::make_shared<Physbuzz::AngularResolver2D>(scene, 0.9f)),
      renderer(window),
      events(*this),
      interface(renderer),
      dynamics(0.005),
      wall(&scene) {}

Game::~Game() {}

void Game::build() {
    glm::ivec2 resolution = glm::ivec2(1920, 1080);
    m_IsRunning = true;

    window.build(resolution);
    renderer.build();
    events.build();
    interface.build();
    collision.build();

    Physbuzz::Context::set(window.getGLFWwindow(), this);

    WallInfo info = {
        .transform = {
            .position = glm::vec3(resolution >> 1, 0.0f),
        },
        .wall = {
            .width = static_cast<float>(resolution.x),
            .height = static_cast<float>(resolution.y),
            .thickness = 10.0f,
        },
        .isCollidable = true,
        .isRenderable = false,
    };

    wall.build(info);
}

void Game::loop() {
    glm::vec4 clear = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    while (m_IsRunning && !window.shouldClose()) {
        window.poll();

        collision.tick(scene);
        dynamics.tick(scene);
        frameClock.tick();

        renderer.clear(clear);
        renderer.render(scene);
        interface.render();

        window.flip();
    }
}

void Game::destroy() {
    m_IsRunning = false;

    for (auto &mesh : scene.getComponents<Physbuzz::RenderComponent>()) {
        mesh.destroy();
    }

    collision.destroy();
    interface.destroy();
    scene.clear();
    events.destroy();
    renderer.destroy();
    window.destroy();
}

int main(int argc, char *argv[]) {
    Game game = Game();

    game.build();
    game.loop();
    game.destroy();

    return 0;
}
