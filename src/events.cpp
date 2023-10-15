#include "events.hpp"
#include "SDL2/SDL.h"
#include "geometry/box.hpp"
#include <cstdio>

EventHandler::EventHandler(Painter *painter) { this->painter = painter; };

void EventHandler::keyboard_keydown(SDL_Event &e) {}

void EventHandler::keyboard_keyup(SDL_Event &e) {}

void EventHandler::mouse_mousedown(SDL_Event &e) {
    AABB *box = new AABB(e.button.x, e.button.y, 10, 10);
    painter->aabb(box);
}
