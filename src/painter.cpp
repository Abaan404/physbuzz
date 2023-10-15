#include "painter.hpp"
#include "SDL_rect.h"
#include "SDL_render.h"
#include <cstdio>

Painter::Painter(SDL_Renderer *renderer) {
    this->renderer = renderer;
}

void Painter::draw() {
    // draw all objects
    for (auto texture = textures.begin(); texture != textures.end(); texture++) {
        Texture txr = *texture;
        SDL_RenderCopy(renderer, txr.texture, NULL, &txr.texture_rect);
    }

    SDL_RenderPresent(renderer);

}

void Painter::background() {
    // Clear the screen
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);
}

void Painter::aabb(AABB *box) {
    // create an AABB box
    SDL_Texture *texture = box->create_texture(renderer, 0, 255, 0, 0);
    SDL_Rect rect = {box->x, box->y, box->width, box->height};

    textures.push_back((Texture){texture, rect});
}
