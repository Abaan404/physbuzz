#pragma once

#include "geometry/box.hpp"
#include "geometry/circle.hpp"
#include "geometry/object.hpp"
#include "imgui_impl_opengl3.h"
#include "opengl/shaders.hpp"
#include <glad/gl.h>
#include <memory>
#include <vector>

typedef glm::ivec4 Color;

class Painter {
  public:
    Painter(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects);

    void render();
    void clear();

    SDL_Texture *draw_box(std::shared_ptr<Box> box, SDL_Color &color);
    SDL_Texture *draw_circle(std::shared_ptr<Circle> circle, SDL_Color &color);

    SDL_Renderer *renderer;
    SDL_GLContext *context;

    SDL_Window *window;
    glm::ivec2 window_size;

    Color clear_color = {1.0f, 1.0f, 1.0f, 0.0f};

  private:
    std::vector<std::shared_ptr<GameObject>> &objects;
};
