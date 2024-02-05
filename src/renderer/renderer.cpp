#include "renderer.hpp"

#include <glad/gl.h>

Renderer::Renderer(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects) : context(context), objects(objects), window(window) {}

void Renderer::render() {
    // clear the frame
    clear({0.0f, 0.0f, 0.0f, 0.0f});

    // draw all objects
    for (auto object : objects) {
        if (object->program == 0) {
            printf("[WARNING] Attempted to render an untextured object\n");
            continue;
        }

        object->draw(this, GL_DYNAMIC_DRAW);
    }

    SDL_GL_SwapWindow(window);
}

void Renderer::clear(Color clear_color) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

// ref: https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
// ref: https://www.youtube.com/watch?v=0K8e5va4QM0 @6:45
// inline void draw_circle_quadrants(SDL_Renderer *renderer, float xc, float yc, float x, float y) {
//     SDL_RenderDrawPoint(renderer, xc + x, yc + y);
//     SDL_RenderDrawPoint(renderer, xc - x, yc + y);
//     SDL_RenderDrawPoint(renderer, xc + x, yc - y);
//     SDL_RenderDrawPoint(renderer, xc - x, yc - y);
//
//     SDL_RenderDrawPoint(renderer, xc + y, yc + x);
//     SDL_RenderDrawPoint(renderer, xc - y, yc + x);
//     SDL_RenderDrawPoint(renderer, xc + y, yc - x);
//     SDL_RenderDrawPoint(renderer, xc - y, yc - x);
// }

// void Renderer::render_circle(std::shared_ptr<Circle> circle) {
//     // float r = circle->radius;
//     // float xc = r;
//     // float yc = r;
//     //
//     // float x = 0;
//     // float y = r;
//     //
//     // // prepare surface for rendering (diameter + 1 pixel for center)
//     // SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, r * 2 + 1, r * 2 + 1);
//     // SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
//     // SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//     // SDL_SetRenderTarget(renderer, texture);
//     //
//     // // draw pixel on the 8 quadrants using bresenhams's thing
//     // draw_circle_quadrants(renderer, xc, yc, x, y);
//     // float p = 3 - (2 * r);
//     // while (x < y) {
//     //     if (p < 0)
//     //         p += 4 * (++x) + 6;
//     //     else
//     //         p += 4 * (++x - --y) + 10;
//     //
//     //     draw_circle_quadrants(renderer, xc, yc, x, y);
//     // }
//     //
//     // // set target to default and cache texture and initial position
//     // SDL_SetRenderTarget(renderer, NULL);
// }

glm::vec3 Renderer::screen_to_world(glm::vec3 vertex) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    return glm::vec3((2.0f * vertex.x) / width - 1.0f, 1.0f - (2.0f * vertex.y) / height, 0);
}

void Renderer::load_object(std::shared_ptr<GameObject> object, GLboolean normalized, GLenum usage) {
    // send data to the gpu
    glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->vertices.size(), object->vertices.data(), usage);

    glBindVertexArray(object->VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, normalized, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * object->indices.size(), object->indices.data(), usage);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
