#pragma once

#include "../geometry/object.hpp"
#include "framebuffer.hpp"
#include <SDL2/SDL.h>

class Renderer {
  public:
    Renderer(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects);

    unsigned int time;
    SDL_Window *window;
    SDL_GLContext *context;

    void render();
    void clear(glm::vec4 &color);
    void target(Framebuffer *framebuffer);
    void resize();

    glm::vec2 get_resolution() { return resolution; };

  private:
    glm::ivec2 resolution;
    Framebuffer *framebuffer;
    std::vector<std::shared_ptr<GameObject>> &objects;
};
