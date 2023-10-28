#include "box.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <glm/glm.hpp>
#include <stdexcept>

Box::Box(int x, int y, int width, int height, Mask mask) {
    // center of the object
    GameObject::x = x;
    GameObject::y = y;

    GameObject::mask = mask;
    this->min = glm::vec2(x - (width >> 1), y - (height >> 1));
    this->max = glm::vec2(x + (width >> 1), y + (height >> 1));
}
