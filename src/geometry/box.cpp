#include "box.hpp"
#include "SDL_render.h"
#include "SDL_surface.h"

AABB::AABB(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height) {}

bool AABB::contains(int px, int py) {
    return (px >= x && px <= (x + width) && py >= y && py <= (y + height));
}

SDL_Texture *AABB::create_texture(SDL_Renderer *renderer, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, Rmask, Gmask, Bmask, Amask);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);
    return texture;
}
