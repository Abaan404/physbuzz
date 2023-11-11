#include "painter.hpp"

void Painter::render() {
    // // draw all objects
    // for (auto object = objects.begin(); object != objects.end(); object++) {
    //     std::shared_ptr<GameObject> obj = *object; // what was i doing
    //     if (obj->texture == NULL) {
    //         printf("[WARNING] Attempted to render an untextured object\n");
    //         continue;
    //     }
    //
    //     // SDL_RenderCopyF(renderer, obj->texture, NULL, &obj->rect);
    // }
    //
    // ImGui widgets drawn in UserInterface::draw()
    // render imgui
    // ImGui::Render();
    // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    //
    // ImGui::UpdatePlatformWindows();
    // ImGui::RenderPlatformWindowsDefault();

    uint32_t image_idx;
    VK_CHECK(vkAcquireNextImageKHR(vk_context.device, vk_context.swapchain, 0, aquire_semaphore, 0, &image_idx));

    VkCommandBufferAllocateInfo cmd_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk_context.command_pool,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(vk_context.device, &cmd_info, &cmd));

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    VkClearValue background_color = {
        .color = {0, 0, 0, 1},
    };

    VkRenderPassBeginInfo renderpass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = vk_context.render_pass,
        .framebuffer = vk_context.framebuffers[image_idx],
        .renderArea = {
            .extent = vk_context.screen_size,
        },
        .clearValueCount = 1,
        .pClearValues = &background_color,
    };
    vkCmdBeginRenderPass(cmd, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    // rendering here
    {

        VkRect2D scissor = {
            .extent = vk_context.screen_size};
        VkViewport viewport = {
            .width = (float)vk_context.screen_size.width,
            .height = (float)vk_context.screen_size.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &aquire_semaphore,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &submit_semaphore,
    };

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &submit_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &vk_context.swapchain,
        .pImageIndices = &image_idx,
    };

    VK_CHECK(vkQueueSubmit(vk_context.queue, 1, &submit_info, 0));
    VK_CHECK(vkQueuePresentKHR(vk_context.queue, &present_info));

    VK_CHECK(vkDeviceWaitIdle(vk_context.device));
    vkFreeCommandBuffers(vk_context.device, vk_context.command_pool, 1, &cmd);
}

void Painter::clear() {
    // Clear the screen
    // if (SDL_RenderClear(renderer) < 0) {
    //     printf("[ERROR] SDL_RenderClear: %s\n", SDL_GetError());
    // }
    //
    // if (SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255) < 0) {
    //     printf("[ERROR] SDL_SetRenderDrawSDL_Color: %s\n", SDL_GetError());
    // }
}

// SDL_Texture *Painter::draw_box(std::shared_ptr<Box> box, SDL_Color &color) {
//     float width = box->max.x - box->min.x;
//     float height = box->max.y - box->min.y;
//
//     // prepare texture for rendering
//     SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width + 1, height + 1);
//     SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
//     SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//     SDL_SetRenderTarget(renderer, texture);
//
//     // draw edges
//     SDL_RenderDrawLine(renderer, 0, 0, width, 0);
//     SDL_RenderDrawLine(renderer, 0, 0, 0, height);
//     SDL_RenderDrawLine(renderer, width, 0, width, height);
//     SDL_RenderDrawLine(renderer, 0, height, width, height);
//
//     // set target to default and cache texture and initial position
//     SDL_SetRenderTarget(renderer, NULL);
//
//     return texture;
// }
//
// // ref: https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
// // ref: https://www.youtube.com/watch?v=0K8e5va4QM0 @6:45
//
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
//
// SDL_Texture *Painter::draw_circle(std::shared_ptr<Circle> circle, SDL_Color &color) {
//     float r = circle->radius;
//     float xc = r;
//     float yc = r;
//
//     float x = 0;
//     float y = r;
//
//     // prepare surface for rendering (diameter + 1 pixel for center)
//     SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, r * 2 + 1, r * 2 + 1);
//     SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
//     SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//     SDL_SetRenderTarget(renderer, texture);
//
//     // draw pixel on the 8 quadrants using bresenhams's thing
//     draw_circle_quadrants(renderer, xc, yc, x, y);
//     float p = 3 - (2 * r);
//     while (x < y) {
//         if (p < 0)
//             p += 4 * (++x) + 6;
//         else
//             p += 4 * (++x - --y) + 10;
//
//         draw_circle_quadrants(renderer, xc, yc, x, y);
//     }
//
//     // set target to default and cache texture and initial position
//     SDL_SetRenderTarget(renderer, NULL);
//     return texture;
// }
