#pragma once

#include "geometry/box.hpp"
#include "geometry/object.hpp"
#include <SDL2/SDL_render.h>
#include <vector>

class Painter {
  public:
    Painter(SDL_Renderer *renderer, std::vector<GameObject *> *objects) : renderer(renderer), objects(objects){};

    void draw();

    void background();
    void render_box(Box *box);

  private:
    SDL_Renderer *renderer;
    std::vector<GameObject *> *objects;
};
