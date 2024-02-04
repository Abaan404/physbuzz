#pragma once

#include "geometry/box.hpp"
#include "geometry/circle.hpp"
#include "geometry/object.hpp"
#include "opengl/shaders.hpp"

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

#include "shaders/box/box.frag"
#include "shaders/box/box.vert"

// need a better color management eventually
typedef glm::ivec4 Color;

// A collection of hard-coded shader for primitive objects
struct ShaderCollection {
    ShaderContext box = ShaderContext(box_vertex, box_frag);
};

class Renderer {
  public:
    Renderer(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects);

    SDL_Window *window;

    ShaderCollection shaders;
    SDL_GLContext *context;
    unsigned int VBO;

    void render();
    void clear(Color clear_color);

    void render_box(std::shared_ptr<Box> box);
    void render_circle(std::shared_ptr<Circle> circle);

  private:
    std::vector<std::shared_ptr<GameObject>> &objects;

    glm::vec3 screen_to_world(glm::vec3 position);
    void load_object(std::shared_ptr<GameObject> object, unsigned char normalized, unsigned int usage);
};
