#pragma once

#include "geometry/box.hpp"
#include "geometry/object.hpp"
#include "store.hpp"
#include <SDL2/SDL_render.h>

class Painter {
  public:
    Painter(SDL_Renderer *renderer, GameObjectStore *storage);

    void draw();

    void background();
    void render_box(Box *box);

  private:
    SDL_Renderer *renderer;
    GameObjectStore *storage;
};
