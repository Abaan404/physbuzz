#include "renderer.hpp"

#include <glad/gl.h>

Renderer::Renderer(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects) : context(context), objects(objects), window(window) {
    target(nullptr);
}

void Renderer::render() {
    time = SDL_GetTicks();

    for (auto object : objects) {
        if (object->texture == nullptr)
            continue;

        object->texture->draw(*this);
    }
}

void Renderer::target(Framebuffer *framebuffer) {
    this->framebuffer = framebuffer;

    if (framebuffer == nullptr) {
        framebuffer->unbind();
        SDL_GetWindowSize(window, &resolution.x, &resolution.y);
        return;
    }

    framebuffer->bind();
    resolution = framebuffer->resolution;
}

void Renderer::clear(glm::vec4 &color) {
    framebuffer->clear(color);
}

void Renderer::resize() {
    if (framebuffer != nullptr)
        framebuffer->resize(resolution);

    SDL_GetWindowSize(window, &resolution.x, &resolution.y);
    glViewport(0, 0, resolution.x, resolution.y);
}
