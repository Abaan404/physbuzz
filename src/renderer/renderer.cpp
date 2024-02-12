#include "renderer.hpp"

#include <glad/gl.h>

Renderer::Renderer(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects) : context(context), objects(objects), window(window) {}

void Renderer::render() {
    // update variables
    SDL_GetWindowSize(window, &width, &height);
    time = SDL_GetTicks();

    // draw all textures
    clear({0.0f, 0.0f, 0.0f, 0.0f});
    for (auto object : objects) {
        if (object->texture == nullptr)
            continue;

        object->texture->draw(*this, GL_DYNAMIC_DRAW);
    }

    SDL_GL_SwapWindow(window);
}

void Renderer::clear(Color clear_color) {
    glViewport(0, 0, width, height);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

glm::vec3 Renderer::screen_to_world(glm::vec3 vertex) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    return glm::vec3((2.0f * vertex.x) / width - 1.0f, 1.0f - (2.0f * vertex.y) / height, 0);
}
