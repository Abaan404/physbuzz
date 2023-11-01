#pragma once

#include "geometry/box.hpp"
#include "geometry/circle.hpp"
#include "geometry/object.hpp"
#include <SDL2/SDL_render.h>
#include <vector>

class Painter {
  public:
    Painter(SDL_Renderer *renderer, std::vector<GameObject *> *objects) : renderer(renderer), objects(objects) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    };

    void draw();

    void render_background();
    void render_box(Box *box);
    void render_circle(Circle *circle);

  private:
    void draw_circle_quadrants(float xc, float yc, float x, float y);

    SDL_Renderer *renderer;
    std::vector<GameObject *> *objects;
};
