#include "painter.hpp"
#include <cstdio>

void Painter::draw() {
    // draw all objects
    for (auto object = objects->begin(); object != objects->end(); object++) {
        GameObject *obj = *object;
        SDL_RenderCopyF(renderer, obj->texture, NULL, &obj->rect);
    }

    SDL_RenderPresent(renderer);
}

void Painter::background() {
    // Clear the screen
    if (SDL_RenderClear(renderer) < 0) {
        printf("[ERROR] SDL_RenderClear: %s\n", SDL_GetError());
        exit(1);
    }

    if (SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0) < 0) {
        printf("[ERROR] SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        exit(1);
    }
}

void Painter::render_box(Box *box) {
    float width = box->max[0] - box->min[0];
    float height = box->max[1] - box->min[1];
    Color color = box->color;

    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, color.R, color.G, color.B, color.A));

    if (surface == NULL) {
        printf("[ERROR] SDL_CreateRGBSurface: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("[ERROR] SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_FreeSurface(surface);

    box->texture = texture;
    box->rect = {box->min[0], box->min[1], width, height};
}
