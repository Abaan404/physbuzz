#include "events.hpp"

#include "objects/objects.hpp"

void EventHandler::keyboard_keydown(SDL_KeyboardEvent &event) {
    switch (event.keysym.scancode | (1 << 30)) {
    case (SDLK_F3):
        interface.draw ^= true;
        break;
    }
}

void EventHandler::keyboard_keyup(SDL_KeyboardEvent &event) {}

void EventHandler::mouse_mousedown(SDL_MouseButtonEvent &event) {
    // FIXME input redirection to ImGui viewport/framebuffer
    if (interface.io.WantCaptureMouse && interface.draw)
        return;

    if (event.button == SDL_BUTTON_LEFT) {
        create_box(scene, glm::vec3(event.x, event.y, 0.0f), 10, 10);
    } else if (event.button == SDL_BUTTON_RIGHT) {
        create_circle(scene, glm::vec3(event.x, event.y, 0.0f), 50);
    }
}
