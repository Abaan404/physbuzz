#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>

struct Vk_context {
    VkAllocationCallbacks *allocator = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t queue_family = (uint32_t)-1;
    VkQueue queue = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT debug_report = VK_NULL_HANDLE;
    VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

    int frame_present();
    int frame_render();
    void cleanup() {
        vkDestroyDevice(device, allocator);
        vkDestroyInstance(instance, allocator);
        vkDestroyDescriptorPool(device, descriptor_pool, allocator);
    }
};

class VulkanBuilder {
  public:
    VulkanBuilder(SDL_Window *window, Vk_context &vk_context) : window(window){};

    void setup();

    void create_instance(std::vector<const char *> instance_extensions);
    VkPhysicalDevice select_physical_device();
    void select_graphics_queue();
    void select_device();
    void create_descriptor_pool();
    void setup_vulkan_window();

    bool is_extension_available(const std::vector<VkExtensionProperties> &properties, const char *extension);
    void check_vk_result(VkResult error);

  private:
    SDL_Window *window;
    Vk_context vk_context;
};
