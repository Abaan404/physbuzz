#include "painter.hpp"

void Painter::render() {
    // draw all objects
    for (auto object = objects.begin(); object != objects.end(); object++) {
        GameObject *obj = &**object; // what was i doing
        if (obj->texture == NULL) {
            printf("[WARNING] Attempted to render an untextured object\n");
            continue;
        }

        SDL_RenderCopyF(renderer, obj->texture, NULL, &obj->rect);
    }

    // ImGui widgets drawn in UserInterface::draw()
    // render imgui
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    SDL_RenderPresent(renderer);
    clear();
}

void Painter::clear() {
    // Clear the screen
    if (SDL_RenderClear(renderer) < 0) {
        printf("[ERROR] SDL_RenderClear: %s\n", SDL_GetError());
    }

    if (SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255) < 0) {
        printf("[ERROR] SDL_SetRenderDrawSDL_Color: %s\n", SDL_GetError());
    }
}

void Painter::draw_box(std::shared_ptr<Box> box) {
    float width = box->max[0] - box->min[0];
    float height = box->max[1] - box->min[1];
    SDL_Color color = box->color;

    // prepare texture for rendering
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width + 1, height + 1);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, texture);

    // draw edges
    SDL_RenderDrawLine(renderer, 0, 0, width, 0);
    SDL_RenderDrawLine(renderer, 0, 0, 0, height);
    SDL_RenderDrawLine(renderer, width, 0, width, height);
    SDL_RenderDrawLine(renderer, 0, height, width, height);

    // set target to default and cache texture and initial position
    SDL_SetRenderTarget(renderer, NULL);
    box->texture = texture;
    box->rect = {box->min[0], box->min[1], width, height};
}

// ref: https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
// ref: https://www.youtube.com/watch?v=0K8e5va4QM0 @6:45
void Painter::draw_circle(std::shared_ptr<Circle> circle) {
    float r = circle->radius;
    float xc = r;
    float yc = r;

    float x = 0;
    float y = r;

    SDL_Color color = circle->color;

    // prepare surface for rendering (diameter + 1 pixel for center)
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, r * 2 + 1, r * 2 + 1);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, texture);

    // draw pixel on the 8 quadrants using bresenhams's thing
    draw_circle_quadrants(xc, yc, x, y);
    float p = 3 - (2 * r);
    while (x < y) {
        if (p < 0)
            p += 4 * (++x) + 6;
        else
            p += 4 * (++x - --y) + 10;

        draw_circle_quadrants(xc, yc, x, y);
    }

    // set target to default and cache texture and initial position
    SDL_SetRenderTarget(renderer, NULL);
    circle->texture = texture;
    circle->rect = {circle->x - r, circle->y - r, 2 * r, 2 * r};
}

void Painter::draw_circle_quadrants(float xc, float yc, float x, float y) {
    SDL_RenderDrawPoint(renderer, xc + x, yc + y);
    SDL_RenderDrawPoint(renderer, xc - x, yc + y);
    SDL_RenderDrawPoint(renderer, xc + x, yc - y);
    SDL_RenderDrawPoint(renderer, xc - x, yc - y);

    SDL_RenderDrawPoint(renderer, xc + y, yc + x);
    SDL_RenderDrawPoint(renderer, xc - y, yc + x);
    SDL_RenderDrawPoint(renderer, xc + y, yc - x);
    SDL_RenderDrawPoint(renderer, xc - y, yc - x);
}
