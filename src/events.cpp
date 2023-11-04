#include "events.hpp"
#include "physics/physics_box.hpp"
#include "physics/physics_circle.hpp"

void EventHandler::keyboard_keydown(SDL_KeyboardEvent &event) {
    switch (event.keysym.scancode | (1 << 30)) {
    case (SDLK_F3):
        printf("F3 pressed!\n");
        break;
    }
}

void EventHandler::keyboard_keyup(SDL_KeyboardEvent &event) {}

void EventHandler::mouse_mousedown(SDL_MouseButtonEvent &event) {
    if (event.button == SDL_BUTTON_LEFT) {
        std::shared_ptr<PhysicsBox> box = std::make_shared<PhysicsBox>(event.x, event.y, 10, 10, (SDL_Color){255, 0, 0, 255});

        painter.render_box(box);
        objects.push_back(box);

    } else if (event.button == SDL_BUTTON_RIGHT) {
        std::shared_ptr<PhysicsCircle> circle = std::make_shared<PhysicsCircle>(event.x, event.y, 100, (SDL_Color){0, 255, 255, 255});

        painter.render_circle(circle);
        objects.push_back(circle);
    }
}
