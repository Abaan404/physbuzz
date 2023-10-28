#include "painter.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

Painter::Painter(SDL_Renderer *renderer, GameObjectStore *storage) : renderer(renderer), storage(storage) {}

void Painter::draw() {
    // draw all objects
    for (auto object = storage->objects.begin(); object != storage->objects.end(); object++) {
        GameObject obj = *object;
        SDL_RenderCopy(renderer, obj.texture, NULL, &obj.rect);
    }

    SDL_RenderPresent(renderer);
}

void Painter::background() {
    // Clear the screen
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);
}

void Painter::render_box(Box *box) {
    int width = box->max[0] - box->min[0];
    int height = box->max[1] - box->min[1];

    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, box->mask.Rmask, box->mask.Gmask, box->mask.Bmask, box->mask.Amask);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    box->texture = texture;
    box->rect = {box->min[0], box->min[1], width, height};
}
