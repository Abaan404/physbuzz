#pragma once

#include "geometry/box.hpp"
#include "geometry/circle.hpp"
#include "geometry/object.hpp"
#include "vulkan.hpp"

#include "imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL_render.h>
#include <memory>
#include <vector>

class Painter {
  public:
    Painter(VulkanContext &vk_context, std::vector<std::shared_ptr<GameObject>> &objects) : vk_context(vk_context), objects(objects) {
        // SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    };

    void render();
    void clear();

    SDL_Texture *draw_box(std::shared_ptr<Box> box, SDL_Color &color);
    SDL_Texture *draw_circle(std::shared_ptr<Circle> circle, SDL_Color &color);

    VulkanContext &vk_context;

  private:
    std::vector<std::shared_ptr<GameObject>> &objects;
};
