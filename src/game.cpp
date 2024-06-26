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
    : collision(std::make_shared<Physbuzz::SeperatingAxis2D>(), std::make_shared<Physbuzz::SeperatingAxis2D>(), std::make_shared<Physbuzz::LinearResolver>(0.9f)),
      renderer(window),
      eventManager(window),
      interface(renderer),
      dynamics(clock),
      wall(&scene) {
    m_IsRunning = true;

    window.build();
    Physbuzz::Context::set(window.getGLFWwindow(), this);

    renderer.build();
    interface.build();

    eventManager.setCallbackKeyEvent(Events::keyEvent);
    eventManager.setCallbackMouseButtonEvent(Events::mouseButton);
    eventManager.setCallbackWindowResize(Events::WindowResize);
    eventManager.setCallbackWindowClose(Events::WindowClose);

    glm::ivec2 resolution = window.getResolution();
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
        eventManager.poll();

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
