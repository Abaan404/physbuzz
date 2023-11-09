#include "imgui_impl_vulkan.h"
#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include <cstdio>
#include <vector>
#include <vulkan/vulkan.h>

struct VulkanContext {
    VkAllocationCallbacks *allocator = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t queue_family = (uint32_t)-1;
    VkQueue queue = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT debug_report = VK_NULL_HANDLE;
    VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

    uint32_t min_image_count = 2;
    bool swapchain_rebuild = false;
    ImGui_ImplVulkanH_Window main_window_data;

    int frame_present();
    int frame_render();
    void cleanup() {
        vkDestroyDevice(device, allocator);
        vkDestroyInstance(instance, allocator);
        vkDestroyDescriptorPool(device, descriptor_pool, allocator);
    }

    static void error_callback(VkResult error) {
        if (error != 0) {
            fprintf(stderr, "[ERROR] vulkan: VkResult = %d\n", error);
            exit(1);
        }
    }

    void rebuild_swapchain(int width, int height) {
        ImGui_ImplVulkan_SetMinImageCount(min_image_count);
        ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physical_device, device, &main_window_data, queue_family, allocator, width, height, min_image_count);
        main_window_data.FrameIndex = 0;
        swapchain_rebuild = false;
    }
};

class VulkanBuilder {
  public:
    VulkanBuilder(SDL_Window *window, VulkanContext &vk_context) : window(window){};

    void setup();

    void create_instance(std::vector<const char *> instance_extensions);
    VkPhysicalDevice select_physical_device();
    void select_graphics_queue();
    void select_device();
    void create_descriptor_pool();

    VkSurfaceKHR create_vulkan_surface();
    void create_frame_buffers(VkSurfaceKHR surface);
    void setup_vulkan_window(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height);

    bool is_extension_available(const std::vector<VkExtensionProperties> &properties, const char *extension);

  private:
    SDL_Window *window;
    VulkanContext vk_context;
};
