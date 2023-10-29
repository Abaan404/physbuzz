#include "events.hpp"
#include <SDL2/SDL.h>

#include "physics/physics_box.hpp"

void EventHandler::keyboard_keydown(SDL_Event &e) {}

void EventHandler::keyboard_keyup(SDL_Event &e) {}

void EventHandler::mouse_mousedown(SDL_Event &e) {
    PhysicsBox *box = new PhysicsBox(e.button.x, e.button.y, 10, 10, (struct Mask){0, 255, 0, 0});

    painter->render_box(box);
    objects->push_back(box);
}
