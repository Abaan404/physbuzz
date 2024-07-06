#include "game.hpp"

#include "events.hpp"
#include "physbuzz/dynamics.hpp"
#include "ui/handler.hpp"
#include <physbuzz/2D/detectors/sepaxis.hpp>
#include <physbuzz/2D/detectors/sweepandprune.hpp>
#include <physbuzz/2D/resolvers/linear.hpp>
#include <physbuzz/context.hpp>
#include <physbuzz/events.hpp>
#include <physbuzz/renderer.hpp>

Game::Game()
    // TODO event management
    : collision(
          std::make_shared<Physbuzz::SeperatingAxis2D>(scene),
          std::make_shared<Physbuzz::SweepAndPrune2D>(scene),
          std::make_shared<Physbuzz::LinearResolver>(scene, 0.9f)),
      renderer(window),
      events(window),
      interface(renderer),
      dynamics(clock),
      wall(&scene) {
    m_IsRunning = true;
    glm::ivec2 resolution = glm::ivec2(1920, 1080);

    window.build(resolution);
    renderer.build();

    // set context before initializing imgui
    Physbuzz::Context::set(window.getGLFWwindow(), this);
    interface.build();

    events.setCallbackKeyEvent(Events::keyEvent);
    events.setCallbackMouseButtonEvent(Events::mouseButton);
    events.setCallbackWindowResizeEvent(Events::WindowResize);
    events.setCallbackWindowCloseEvent(Events::WindowClose);

    collision.build();

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
        events.poll();

        collision.tick(scene);
        dynamics.tick(scene);
        clock.tick();

        renderer.clear(clear);
        renderer.render(scene);
        interface.render();

        window.flip();
    }
}

Game::~Game() {
    m_IsRunning = false;

    for (auto &mesh : scene.getComponents<Physbuzz::RenderComponent>()) {
        mesh.destroy();
    }

    scene.clear();
    renderer.destroy();
    window.destroy();
}

int main(int argc, char *argv[]) {
    Game game = Game();
    game.loop();

    return 0;
}
