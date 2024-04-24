#pragma once

#include "../scene/scene.hpp"
#include "mesh.hpp"
#include "framebuffer.hpp"
#include <SDL2/SDL.h>
#include <glad/gl.h>

class Renderer {
  public:
    Renderer(SDL_GLContext *context, SDL_Window *window, Scene &scene);

    unsigned int time;
    SDL_Window *window;
    SDL_GLContext *context;

    void render();
    void target(Framebuffer *framebuffer);
    void clear(glm::vec4 color);
    void resize(glm::ivec2 resolution);

    glm::vec2 get_resolution() { return resolution; };

  private:
    glm::ivec2 resolution;
    Framebuffer *framebuffer;
    Scene &scene;

    std::vector<float> normalize_vertex(MeshComponent &mesh);
    void render_mesh(MeshComponent &mesh);
};
