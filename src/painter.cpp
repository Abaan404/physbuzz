#include "painter.hpp"
#include "SDL_rect.h"
#include "SDL_render.h"
#include <cstdio>

Painter::Painter(SDL_Renderer *renderer) {
    this->renderer = renderer;
}

void Painter::draw() {
    // draw all objects
    for (auto object = objects.begin(); object != objects.end(); object++) {
        Object *obj = *object;
        SDL_RenderCopy(renderer, obj->texture, NULL, &obj->rect);
    }

    SDL_RenderPresent(renderer);
}

void Painter::background() {
    // Clear the screen
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);
}

void Painter::aabb(AABB *box) {
    box->render_texture(renderer, 0, 255, 0, 0);
    objects.push_back(box);
}
