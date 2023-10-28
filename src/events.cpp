#include "events.hpp"
#include <SDL2/SDL.h>

#include "physics/physics_box.hpp"
#include "store.hpp"

EventHandler::EventHandler(Painter *painter, GameObjectStore *storage) : painter(painter), storage(storage){};

void EventHandler::keyboard_keydown(SDL_Event &e) {}

void EventHandler::keyboard_keyup(SDL_Event &e) {}

void EventHandler::mouse_mousedown(SDL_Event &e) {
    PhysicsBox box = PhysicsBox(e.button.x, e.button.y, 10, 10, (struct Mask){0, 255, 0, 0});
    painter->render_box(&box);

    storage->push_object(box);
}
