#include "painter.hpp"

Painter::Painter(SDL_GLContext *context, SDL_Window *window, std::vector<std::shared_ptr<GameObject>> &objects) : context(context), objects(objects), window(window) {
    glGenBuffers(1, &VBO);
}

void Painter::render() {
    // draw all objects
    for (auto object = objects.begin(); object != objects.end(); object++) {
        GameObject obj = **object;
        if (obj.program == 0) {
            printf("[WARNING] Attempted to render an untextured object\n");
            continue;
        }

        glUseProgram(obj.program);
        glBindVertexArray(obj.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.EBO);
        glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
    }

    SDL_GL_SwapWindow(window);

    // clear the frame
    clear({1.0f, 0.0f, 1.0f, 0.0f});
}

void Painter::clear(Color clear_color) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Painter::render_box(std::shared_ptr<Box> box) {
    std::vector<glm::vec3> vertex_buffer = {
        glm::vec3(0.5f, 0.5f, 0.0f),
        glm::vec3(0.5f, -0.5f, 0.0f),
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3(-0.5f, 0.5f, 0.0f),
    };

    std::vector<glm::uvec3> index_buffer = {
        glm::vec3(0, 1, 3),
        glm::vec3(1, 2, 3),
    };

    box->program = shaders.box.load();
    box->set_vertex(vertex_buffer);
    box->set_index(index_buffer);

    load_object(box, GL_FALSE, GL_STATIC_DRAW);

    // float width = box->max.x - box->min.x;
    // float height = box->max.y - box->min.y;
    //
    // // prepare texture for rendering
    // SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width + 1, height + 1);
    // SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    // SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    // SDL_SetRenderTarget(renderer, texture);
    //
    // // draw edges
    // SDL_RenderDrawLine(renderer, 0, 0, width, 0);
    // SDL_RenderDrawLine(renderer, 0, 0, 0, height);
    // SDL_RenderDrawLine(renderer, width, 0, width, height);
    // SDL_RenderDrawLine(renderer, 0, height, width, height);
    //
    // // set target to default and cache texture and initial position
    // SDL_SetRenderTarget(renderer, NULL);
    //
    // return texture;
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

void Painter::render_circle(std::shared_ptr<Circle> circle) {
    // float r = circle->radius;
    // float xc = r;
    // float yc = r;
    //
    // float x = 0;
    // float y = r;
    //
    // // prepare surface for rendering (diameter + 1 pixel for center)
    // SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, r * 2 + 1, r * 2 + 1);
    // SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    // SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    // SDL_SetRenderTarget(renderer, texture);
    //
    // // draw pixel on the 8 quadrants using bresenhams's thing
    // draw_circle_quadrants(renderer, xc, yc, x, y);
    // float p = 3 - (2 * r);
    // while (x < y) {
    //     if (p < 0)
    //         p += 4 * (++x) + 6;
    //     else
    //         p += 4 * (++x - --y) + 10;
    //
    //     draw_circle_quadrants(renderer, xc, yc, x, y);
    // }
    //
    // // set target to default and cache texture and initial position
    // SDL_SetRenderTarget(renderer, NULL);
}

void Painter::load_object(std::shared_ptr<GameObject> object, GLboolean normalized, GLenum usage) {
    // send data to the gpu
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * object->vertices.size(), object->vertices.data(), usage);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * object->indices.size(), object->indices.data(), usage);

    glVertexAttribPointer(0, 3, GL_FLOAT, normalized, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
