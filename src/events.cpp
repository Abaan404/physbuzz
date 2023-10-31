#include "events.hpp"
#include <SDL2/SDL.h>

#include "physics/physics_box.hpp"
#include "physics/physics_circle.hpp"

void EventHandler::keyboard_keydown(SDL_Event &e) {}

void EventHandler::keyboard_keyup(SDL_Event &e) {}

void EventHandler::mouse_mousedown(SDL_MouseButtonEvent &e) {
    if (e.button == SDL_BUTTON_LEFT) {
        PhysicsBox *box = new PhysicsBox(e.x, e.y, 10, 10, (struct Color){255, 0, 0, 0});

        painter->render_box(box);
        objects->push_back(box);

    } else if (e.button == SDL_BUTTON_RIGHT) {
        PhysicsCircle *circle = new PhysicsCircle(e.x, e.y, 10, (struct Color){0, 255, 255, 0});

        // painter->render_circle(circle);
        objects->push_back(circle);
    }
}
