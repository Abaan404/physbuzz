#pragma once

#include "SDL_render.h"
#include "geometry/box.hpp"
#include "geometry/object.hpp"
#include <list>

class Painter {
  public:
    Painter(SDL_Renderer *renderer);

    void draw();

    void background();
    void aabb(AABB *box);

  private:
    SDL_Renderer *renderer;
    std::list<Object *> objects;
};
