#pragma once

#include "geometry/box.hpp"
#include "geometry/object.hpp"
#include <SDL2/SDL_render.h>
#include <list>

class Painter {
  public:
    Painter(SDL_Renderer *renderer, std::list<Object *> *objects);

    void draw();

    void background();
    void aabb(AABB *box);

  private:
    SDL_Renderer *renderer;
    std::list<Object *> *objects;
};
