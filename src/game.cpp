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
}

void Game::loop() {
    glm::vec4 clear = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    while (isRunning) {
        eventManager.poll();
        dynamics.tick(scene);

        renderer.clear(clear);
        renderer.render(scene);
        interface.render();
        window.flip();
    }
}

Game::~Game() {
    std::vector<Physbuzz::MeshComponent> &meshes = scene.getComponents<Physbuzz::MeshComponent>();

    for (auto &mesh : meshes) {
        mesh.destroy();
    }
}

int main(int argc, char *argv[]) {
    Game game = Game();
    game.loop();

    return 0;
}
