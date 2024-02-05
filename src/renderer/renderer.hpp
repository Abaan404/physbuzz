#pragma once

#include "../geometry/object.hpp"

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

// need a better color management eventually
typedef glm::vec4 Color;

class Renderer {
  public:
    Renderer(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects);

    SDL_Window *window;

    SDL_GLContext *context;

    void render();
    void clear(Color clear_color);
    
    // helper method(s)
    glm::vec3 screen_to_world(glm::vec3 position);

  private:
    std::vector<std::shared_ptr<GameObject>> &objects;

    void load_object(std::shared_ptr<GameObject> object, unsigned char normalized, unsigned int usage);
};
