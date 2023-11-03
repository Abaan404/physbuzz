#include "events.hpp"
#include <SDL2/SDL.h>
#include <cstdio>

#include "physics/physics_box.hpp"
#include "physics/physics_circle.hpp"

void EventHandler::keyboard_keydown(SDL_KeyboardEvent &e) {
    switch (e.keysym.scancode | (1 << 30)) {
        case (SDLK_F3):
            printf("F3 pressed!\n");
            break;
    }
}

void EventHandler::keyboard_keyup(SDL_KeyboardEvent &e) {}

void EventHandler::mouse_mousedown(SDL_MouseButtonEvent &e) {
    if (e.button == SDL_BUTTON_LEFT) {
        PhysicsBox *box = new PhysicsBox(e.x, e.y, 10, 10, (SDL_Color){255, 0, 0, 255});

        painter->render_box(box);
        objects->push_back(box);

    } else if (e.button == SDL_BUTTON_RIGHT) {
        PhysicsCircle *circle = new PhysicsCircle(e.x, e.y, 100, (SDL_Color){0, 255, 255, 255});

        painter->render_circle(circle);
        objects->push_back(circle);
    }
}
