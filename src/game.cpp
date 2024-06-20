#include "game.hpp"

#include "events.hpp"
#include <physbuzz/events.hpp>
#include <physbuzz/renderer.hpp>

Game::Game() {
    isRunning = true;

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

    while (isRunning) {
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
    for (auto &mesh : scene.getComponents<Physbuzz::RenderComponent>()) {
        mesh.destroy();
    }
}

int main(int argc, char *argv[]) {
    Game game = Game();
    game.loop();

    return 0;
}
