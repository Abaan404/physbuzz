#include "vulkan.hpp"
#include <cstdio>
#include <cstdlib>

// ref: imgui vulkan-sdl2 example
// could have tried to do this myself but
// there is so much boilerplate, im better
// off copying imgui's bootstrap example and
// returning to this if something breaks

// psa learning how to setup vulkan at 3am is not a good idea

void VulkanBuilder::check_vk_result(VkResult error) {
    if (error != 0) {
        fprintf(stderr, "[ERROR] vulkan: VkResult = %d\n", error);
        exit(1);
    }
}

void VulkanBuilder::setup() {

    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, nullptr);

    std::vector<const char *> extensions(extensions_count);
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, extensions.data());

    create_instance(extensions);
    select_physical_device();
    select_device();
    select_graphics_queue();
    create_descriptor_pool();
}

bool VulkanBuilder::is_extension_available(const std::vector<VkExtensionProperties> &properties, const char *extension) {
    for (const VkExtensionProperties &p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

void VulkanBuilder::create_instance(std::vector<const char *> instance_extensions) {
    VkResult error;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Enumerate available extensions
    uint32_t properties_count;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    error = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
    check_vk_result(error);

    // Enable required extensions
    if (is_extension_available(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
        instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (is_extension_available(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    // Create Vulkan Instance
    create_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
    create_info.ppEnabledExtensionNames = instance_extensions.data();
    error = vkCreateInstance(&create_info, vk_context.allocator, &vk_context.instance);
    check_vk_result(error);
}

VkPhysicalDevice VulkanBuilder::select_physical_device() {
    uint32_t gpu_count;
    VkResult err = vkEnumeratePhysicalDevices(vk_context.instance, &gpu_count, nullptr);
    check_vk_result(err);
    if (gpu_count <= 0) {
        printf("[ERROR] No GPUs found\n");
        exit(1);
    }

    std::vector<VkPhysicalDevice> gpus;
    gpus.resize(gpu_count);
    err = vkEnumeratePhysicalDevices(vk_context.instance, &gpu_count, gpus.data());
    check_vk_result(err);

    // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
    // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
    // dedicated GPUs) is out of scope of this sample.
    for (VkPhysicalDevice &device : gpus) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return device;
    }

    // Use first GPU (Integrated) is a Discrete one is not available.
    if (gpu_count > 0)
        return gpus[0];
    return VK_NULL_HANDLE;
}

void VulkanBuilder::select_graphics_queue() {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context.physical_device, &count, nullptr);
    VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context.physical_device, &count, queues);
    for (uint32_t i = 0; i < count; i++)
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vk_context.queue_family = i;
            break;
        }
    free(queues);

    if (vk_context.queue_family == (uint32_t)-1) {
        printf("[ERROR] Could not select a graphics queue familiy\n");
        exit(1);
    }
}

void VulkanBuilder::select_device() {
    VkResult error;
    std::vector<const char *> device_extensions;
    device_extensions.push_back("VK_KHR_swapchain");

    // Enumerate physical device extension
    uint32_t properties_count;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(vk_context.physical_device, nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    vkEnumerateDeviceExtensionProperties(vk_context.physical_device, nullptr, &properties_count, properties.data());

#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = vk_context.queue_family;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();
    error = vkCreateDevice(vk_context.physical_device, &create_info, vk_context.allocator, &vk_context.device);
    check_vk_result(error);
    vkGetDeviceQueue(vk_context.device, vk_context.queue_family, 0, &vk_context.queue);
}

void VulkanBuilder::create_descriptor_pool() {
    VkResult error;
    VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize));
    pool_info.pPoolSizes = pool_sizes;
    error = vkCreateDescriptorPool(vk_context.device, &pool_info, vk_context.allocator, &vk_context.descriptor_pool);
    check_vk_result(error);
}
