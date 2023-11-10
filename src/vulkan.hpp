#pragma once

#define VULKAN_DEBUG_REPORT

#include "imgui_impl_vulkan.h"
#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <vulkan/vulkan.h>

#define VK_CHECK(result)                 \
    if (result != VK_SUCCESS) {          \
        printf("[Vulkan] %d\n", result); \
        __builtin_trap();                \
        exit(1);                         \
    }

#ifdef DEBUG
#define VULKAN_DEBUG_REPORT
#endif

struct VulkanContext {
    VkAllocationCallbacks *allocator = nullptr;
    VkInstance instance;

#ifdef VULKAN_DEBUG_REPORT
    VkDebugReportCallbackEXT debug_report;
#endif

    VkPhysicalDevice physical_device;
    VkDevice device;
    uint32_t queue_family = (uint32_t)-1;
    VkQueue queue;

    VkSurfaceFormatKHR surface_format;
    VkSurfaceKHR surface;

    VkPipelineCache pipeline_cache;
    VkDescriptorPool descriptor_pool;

    VkSwapchainKHR swapchain;
    VkCommandPool command_pool;

    uint32_t sc_image_count;
    uint32_t min_image_count = 2;
    std::vector<VkImage> sc_images;

    bool swapchain_rebuild = false;
    ImGui_ImplVulkanH_Window main_window_data;

    void frame_present();
    void frame_render(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data);
    void rebuild_swapchain(int width, int height);

    void cleanup();

    static void error_callback(VkResult error) {
        if (error != 0) {
            fprintf(stderr, "[ERROR] vulkan: VkResult = %d\n", error);
            exit(1);
        }
    }
};

class VulkanBuilder {
  public:
    VulkanBuilder(SDL_Window *window, VulkanContext *vk_context) : window(window), vk_context(vk_context){};

    void setup();

    void create_instance(std::vector<const char *> instance_extensions);
    VkPhysicalDevice select_physical_device();
    void select_graphics_queue_family();
    void select_device(std::vector<const char*> device_extensions);
    void create_descriptor_pool();

    void create_vulkan_surface();
    void create_frame_buffers();
    void setup_vulkan_window(ImGui_ImplVulkanH_Window *wd, int width, int height);

    void create_swapchain();
    void create_command_pool();

    bool is_extension_available(const std::vector<VkExtensionProperties> &properties, const char *extension);

  private:
    SDL_Window *window;
    VulkanContext *vk_context;
};
