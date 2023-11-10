#pragma once

#include "geometry/box.hpp"
#include "geometry/circle.hpp"
#include "geometry/object.hpp"
#include "vulkan.hpp"

#include "imgui_impl_sdlrenderer2.h"
#include <memory>
#include <vector>

class Painter {
  public:
    Painter(VulkanContext &vk_context, std::vector<std::shared_ptr<GameObject>> &objects) : vk_context(vk_context), objects(objects) {
        VkSemaphoreCreateInfo semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        VK_CHECK(vkCreateSemaphore(vk_context.device, &semaphore_info, vk_context.allocator, &aquire_semaphore));
        VK_CHECK(vkCreateSemaphore(vk_context.device, &semaphore_info, vk_context.allocator, &submit_semaphore));
    };

    void render();
    void clear();

    SDL_Texture *draw_box(std::shared_ptr<Box> box, SDL_Color &color);
    SDL_Texture *draw_circle(std::shared_ptr<Circle> circle, SDL_Color &color);

    VulkanContext &vk_context;

  private:
    VkSemaphore submit_semaphore;
    VkSemaphore aquire_semaphore;

    std::vector<std::shared_ptr<GameObject>> &objects;
};
