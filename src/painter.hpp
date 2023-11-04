#pragma once

#include "geometry/box.hpp"
#include "geometry/circle.hpp"
#include "geometry/object.hpp"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL_render.h>
#include <memory>
#include <vector>

class Painter {
  public:
    Painter(SDL_Renderer *renderer, std::vector<std::shared_ptr<GameObject>> &objects) : renderer(renderer), objects(objects) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    };

    void render();
    void clear();

    void render_box(std::shared_ptr<Box> box);
    void render_circle(std::shared_ptr<Circle> circle);

    SDL_Renderer *renderer;

  private:
    void draw_circle_quadrants(float xc, float yc, float x, float y);

    std::vector<std::shared_ptr<GameObject>> &objects;
};
