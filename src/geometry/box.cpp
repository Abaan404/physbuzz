#include "box.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <glm/glm.hpp>

AABB::AABB(int x, int y, int width, int height, Mask mask) {
    // center of the object
    Object::x = x;
    Object::y = y;

    Object::mask = mask;
    this->min = glm::vec2(x - (width >> 1), y - (height >> 1));
    this->max = glm::vec2(x + (width >> 1), y + (height >> 1));
}

// ref: https://code.tutsplus.com/collision-detection-using-the-separating-axis-theorem--gamedev-169t
// currently just assumes every box is perpendicular for now
bool AABB::collides(AABB *aabb) {

    // if not colliding on any axis
    if (this->max[0] < aabb->min[0] or this->min[0] > aabb->max[0])
        return false;

    if (this->max[1] < aabb->min[1] or this->min[1] > aabb->max[1])
        return false;

    return true;
}
