#pragma once

#include "SDL_render.h"
#include "geometry/box.hpp"
#include <list>

class Texture {
  public:
    SDL_Texture *texture;
    SDL_Rect texture_rect; // store by value for now, pointers kept me awake
                           // until 2:29am
};

class Painter {
  public:
    Painter(SDL_Renderer *renderer);

    SDL_Renderer *renderer;

    void draw();

    void background();
    void aabb(AABB *box);

  private:
    std::list<Texture> textures;
};
