#include "game.hpp"

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
    builder.build();
    bindings.build();
    interface.build(window);
    collision.build();

    window.addCallback<Physbuzz::WindowResizeEvent>([&](const Physbuzz::WindowResizeEvent &event) {
        player.resize(event.resolution);
        renderer.renderer.resize(event.resolution);
    });

    window.addCallback<Physbuzz::WindowCloseEvent>([&](const Physbuzz::WindowCloseEvent &event) {
        window.close();
    });
}

void Game::loop() {
    m_IsRunning = true;
    renderer.activeCamera = &player.camera;

    while (m_IsRunning && !window.shouldClose()) {
        window.poll();

        // collision.tick(scene);
        // dynamics.tick(scene);
        bindings.tick();

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
    bindings.destory();
    scene.clear();
    builder.destroy();
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
