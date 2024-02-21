#include "events.hpp"

#include "geometry/box/box.hpp"
#include "geometry/circle/circle.hpp"

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
        std::shared_ptr<Box> box = std::make_shared<Box>(glm::vec2(event.x, event.y), 10, 10, 1.0f);
        box->dynamics->velocity = glm::vec2(-.05f, -.01f);

        objects.push_back(box);

    } else if (event.button == SDL_BUTTON_RIGHT) {
        std::shared_ptr<Circle> circle = std::make_shared<Circle>(glm::vec2(event.x, event.y), 100, 1.0f);
        circle->dynamics->velocity = glm::vec2(.1f, .1f);

        objects.push_back(circle);
    }
}
