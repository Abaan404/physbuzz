#include "box.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

AABB::AABB(int x, int y, int width, int height, Mask mask) {
    Object::x = x;
    Object::y = y;
    Object::mask = mask;

    this->width = width;
    this->height = height;
}

bool AABB::contains(int px, int py) {
    return (px >= x && px <= (x + width) && py >= y && py <= (y + height));
}
