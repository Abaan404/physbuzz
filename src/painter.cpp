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
    SDL_Surface *surface = SDL_CreateRGBSurface(0, box->width, box->height, 32, box->mask.Rmask, box->mask.Gmask, box->mask.Bmask, box->mask.Amask);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    box->texture = texture;
    box->rect = {box->x, box->y, box->width, box->height};

    objects.push_back(box);
}
